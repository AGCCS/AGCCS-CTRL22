/*
*************************************************************************
*************************************************************************
*************************************************************************

agccs ctrl22 -- 2021.02.20

- target platform agccs board rev 1.2
- compiles with avr-gcc 
- requires avr-libc 1.x.x and gcc 7.3
- although this is a complete re-write, this project benefits from earlier work
  (a) the SmartEVSE by Michael Stegen (https://github.com/SmartEVSE) and
  (b) Pascal Thurnherr's Bachelor Thesis (https://github.com/dreadnomad/FGCCS-Ctrl22)
- this software is (C) Thomas Moor 2020 and is distributed by the same license terms
  as the projects (a) and (b) above. 

revisions since first release
- xxx


*************************************************************************
*************************************************************************
*************************************************************************

Copyright THomas Moor 2020-2021

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


*************************************************************************
*************************************************************************
*************************************************************************
*/


// firmware version
#define CTRL22_VERSION 12  // XY reads vX.Y


// 10 MHz clock on fgccs board 
// (running on different clock requires careful re-examination  of code) 
#ifndef F_CPU
#define F_CPU 10000000UL
#endif

// std includes
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


// have version as parameter for easy read out
const int16_t g_version=CTRL22_VERSION;   

/*
*************************************************************************
*************************************************************************
*************************************************************************

calibration and configuration (defaults can be saved/loaded to/from eeprom)

- calibration parameters for AC current measurement [not yet implemented]
- configuration data for max load
- configuration data for enabled phases
- configuration data for optional lock
 
*************************************************************************
*************************************************************************
*************************************************************************
*/

#define PARCNT 4          // number of parameters
int16_t p_smaxcur=160;    // max current in [100mA]
int16_t p_phases=123;     // enabled phases
int16_t p_lclosems=100;   // ms to close lock (set to 0 for no lock)
int16_t p_lopenms=100;    // ms to open lock  (set to 0 for no lock)



/*
*************************************************************************
*************************************************************************
*************************************************************************

convenience/debugging macros

- choose availabe modes of operation
- pragmatic fixed point maths

*************************************************************************
*************************************************************************
*************************************************************************
*/


// code section select switches
#define MODE_DEVELOP                         // enable mode for code/hardware tests
#define MODE_CONFIGURE                       // enable mode for calibration/configuration 


// serial debug should write synchronous
#define DEBUG_WRITE(c)  {serial_writeln_sync(); c; serial_writeln_async();}

// debug command line interface
//#define DEBUG_CLI
#ifdef DEBUG_CLI
#define DBW_CLI(c)  DEBUG_WRITE(c)
#define DEBUG
#else
#define DBW_CLI(c)
#endif

// debug CCS charger state
#define DEBUG_CCS
#ifdef DEBUG_CCS
#define DBW_CCS(c)  DEBUG_WRITE(c)
#define DEBUG
#else
#define DBW_CCS(c)
#endif

// debug analog reading
#define DEBUG_ADC
#ifdef DEBUG_ADC
#define DBW_ADC(c)  DEBUG_WRITE(c)
#define DEBUG
#else
#define DBW_ADC(c)
#endif

// debug rms measurement
#define DEBUG_RMS
#ifdef DEBUG_RMS
#define DEBUG
#endif

// debug nvm access
//#define DEBUG_NVM
#ifdef DEBUG_NVM
#define DEBUG
#endif


// debug lock
#define DEBUG_LOCK
#ifdef DEBUG_LOCK
#define DBW_LOCK(c)  DEBUG_WRITE(c)
#define DEBUG
#else
#define DBW_LOCK(c)
#endif


// forward declaration of time/ticks (see section systime)
uint16_t g_systicks;
int16_t g_systime;
uint16_t g_cycleskip;

// mutex for shared resource (set/rel in main loop or call backs, not in ISRs)
char g_adc0_bsy=false;

// convenience: return true if duetime has expired (uint16 ticks)
#define TRIGGER_SCHEDULE(duetime)  (   \
   ((g_systicks >= duetime) &&  (g_systicks-duetime < 0x8000)) ||  \
   ((g_systicks < duetime) &&  (duetime-g_systicks > 0x8000)) )

// forward declaration of ccs charging state (see main loop)
typedef enum {OFF0=00,OFF1=01,A0=10,A1=11,B0=20,B1=21,C0=30,C1=31,C2=32,P0=41,P1=42,W0=50,W1=51,ERR0=0x70} ccs_state_t;
ccs_state_t g_ccs_st=OFF0;

// cli veraiant of ccs state (how do we properly/safly cast an enum to an int16_t?)
int16_t g_ccss_cli=0;

// forward declaration of error code
int16_t g_error=0;
#define ERR_LOCK    0x0001    // lock jammed
#define ERR_CCS     0x0002    // CCS protocol error
#define ERR_CONF    0x0004    // faild to read configuration


/*
*************************************************************************
*************************************************************************
*************************************************************************

component initialisation/access

- xmega basics
- eeprom access
- fgccs led/button (ui) 
- serial line on uart0 (configuration/development)
- rms adc (current measurement)
- adc for pilot/temp readings
- systime

*************************************************************************
*************************************************************************
*************************************************************************
*/


/*
*************************************************************************
xmega basics
*************************************************************************
*/


// clock select 10 MHz
void clock_init(void) {
  CPU_CCP = CCP_IOREG_gc;          
  CLKCTRL.MCLKCTRLB = (CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm); // devide internal 20MHz osc by 2    
  while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);
}

// software reset (cli wrapper)
int16_t reset(int16_t val) {
  if(!val) return 0;
  CPU_CCP = CCP_IOREG_gc;          
  RSTCTRL.SWRR|=RSTCTRL_SWRE_bm;
  return 1;
}  


/*
*************************************************************************
led/button
- callback for synchronized blink-codes
- callback for button edge detection
*************************************************************************
*/

// the led is either on PD3 or PF0, depending on the board
#define LED_PD3
//define LED_PF0

void ledbutton_init(void) {
  PORTD.DIRCLR = PIN4_bm; // button
#ifdef LED_PF0
  PORTF.DIRSET = PIN0_bm; // led, var 1
#endif  
#ifdef LED_PD3
  PORTD.DIRSET = PIN3_bm; // led, var 2
#endif  
}    

inline void led_on() {
#ifdef LED_PF0
  PORTF.OUTSET = PIN0_bm;
#endif  
#ifdef LED_PD3
  PORTD.OUTSET = PIN3_bm;
#endif  
}

inline void led_off() {
#ifdef LED_PF0
  PORTF.OUTCLR = PIN0_bm;
#endif  
#ifdef LED_PD3
  PORTD.OUTCLR = PIN3_bm;
#endif  
}


// number of flashs per 2sec period (range 1-20)
int16_t g_blinks=1;

// extra flash patterns
#define BLINKS_ON    21
#define BLINKS_OFF    0
#define BLINKS_HEART  1
#define BLINKS_RELAX 22
#define BLINKS_ERR   20

// flash my led (callback in main loop)
void led_blinks_cb(void) {
  // const on
  if(g_blinks==BLINKS_ON) {
    led_on();
    return;
  }
  // const off
  if(g_blinks==BLINKS_OFF) {
    led_off();
    return;
  }
  // sync on 2sec with systime (30sec rollover) 
  int16_t led_time=g_systime % 2000;
  int16_t led_cycle;
  // fast cycle, range 1-20, incl BLINKS_ERR
  if(g_blinks<=20) {
    led_cycle = led_time / 100 +1;
    if(led_cycle > g_blinks) {
      led_off();
      return;
    }  
    if(led_time % 100 <50)
      led_on();
    else
      led_off();
  }
  // slow cycle, range 1-4, for BLINK_RELAX
  if(g_blinks==BLINKS_RELAX) {
    led_cycle = led_time / 500 +1;
    if(led_cycle > 4) {
      led_off();
      return;
    }  
    if(led_time % 500 <250)
      led_on();
    else
      led_off();
  }  
}

// button pressed (positive edge detection, incl debounce, user must reset)
bool g_button=false;

// button sense callback
void button_cb(void) {
  g_button=false;
  static bool recent=false;
  static char cnt=0;
  bool now = PORTD.IN & PIN4_bm;
  if(now==recent) cnt=0;
  else cnt++;
  if(cnt<50) return;
  recent=now;
  cnt=0;
  if(now) g_button=true;
  return;
}  
  


/*
*************************************************************************
serial line on uart0
-- write buffered line asynchronously 
-- read buffered line asynchronously 
-- effectively support a line-by-line serial protocol
*************************************************************************
*/

// baudrate formula from atmel documentation
#define USART_BAUD_RATE(BAUD_RATE) (uint16_t) ( (F_CPU * 64.0) / (16.0 * BAUD_RATE) + 0.5 ) 

// component: init serial line on usart0 qith pins PA0 (TX) and PA1 (RX)
void serial_init(void) {
  USART0.BAUD  = USART_BAUD_RATE(115200);                                // set baud rate
  USART0.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;         // 8 data bits, one stop bit, no parity
  USART0.CTRLA |= USART_RXCIE_bm;                                        // enable RX interupt
  PORTA.DIRSET = PIN0_bm;                                                // set TX pin
  PORTA.DIRCLR = PIN1_bm;                                                // set RX pin
  USART0.CTRLB |= (USART_TXEN_bm | USART_RXEN_bm);                       // endable uart
}

// linebuffer
#define IOBUFFLEN  64              // buffer size
char g_writeln_buf[IOBUFFLEN+1];   // actual buffer (+1 for terminating 0, +2 for "\r\n")
unsigned char g_writeln_pos=0;     // currrent position to write to
volatile char g_writeln_st=0;      // state: 0<>writing to buffer; 1<>sending via uart
bool g_writeln_sync=false;         // true<>synchronous mode; false<>asynchronous mode

// interrupt service routine to send characters from 0-terminated buffer
ISR(USART0_DRE_vect) {
  char data=g_writeln_buf[g_writeln_pos++];          // get data from buffer
  if((data=='\0') || (g_writeln_pos>=IOBUFFLEN)) {   // sense end of string
    USART0.CTRLA  &= ~USART_DREIE_bm;                // disable DRE interupt
    g_writeln_pos=0;
    g_writeln_st=0;
    return;
  }  
  USART0_TXDATAL=data;                               // write byte to data register
}  
    
// write one byte to line buffer
void serial_write(const unsigned char data){
  if(g_writeln_st!=0) return;
  if(g_writeln_pos<IOBUFFLEN) g_writeln_buf[g_writeln_pos++]=data;
}

// write end of line and start transmission
void serial_write_eol(void){
  // device busy
  if(g_writeln_st!=0) return;
  // terminate line
  g_writeln_buf[g_writeln_pos++]='\r';
  g_writeln_buf[g_writeln_pos++]='\n';
  g_writeln_buf[g_writeln_pos++]=0;
  // send via ISR
  g_writeln_pos=0;  
  g_writeln_st=1;
  while(!(USART0_STATUS & USART_DREIF_bm));
  USART0.CTRLA |= USART_DREIE_bm;    // enable DRE interupt
  // if in synchronous mode, wait until transmision is complete
  if(g_writeln_sync)
    while(g_writeln_st!=0);  
}

// set to synchronous mode (block on eol until transmission is complete)
void serial_writeln_sync(void) {
  while(g_writeln_st!=0);
  g_writeln_sync=true;
  g_cycleskip=true;
}  

// set to asynchronous mode (do not block)
void serial_writeln_async(void) {
  while(g_writeln_st!=0);
  g_writeln_sync=false;
  g_cycleskip=true;
}  

// convenience: write 0-terminated string 
void serial_write_str(const char* str){
  if(g_writeln_st!=0) return;
  while((*str!='\0') && (g_writeln_pos<IOBUFFLEN))
    g_writeln_buf[g_writeln_pos++]=*(str++);
}

// convenience: write 0-terminated string as line
void serial_writeln(const char* str){
  serial_write_str(str);
  serial_write_eol();
}

// convenience: write unsigned integer (16bit) 
void serial_write_uint(uint16_t value) {
  // device busy
  if(g_writeln_st!=0) return;
  // eliminate singular case '0'
  if(value==0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0';
    return;
  }
  // figure number of digits
  uint16_t divisor=1;
  while(value>=divisor*10) {
    divisor=divisor*10;
    if(divisor>=10000) break; // max 64K >> divisor max 10K
  }  
  // print digits  
  while(divisor>0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0'+value/divisor;
    value=value % divisor;
    divisor = divisor/10;
  }  
}    

// convenience: write integer (16bit) 
void serial_write_int(int16_t value) {
  // device busy
  if(g_writeln_st!=0) return;
  // eliminate singular case '0'
  if(value==0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0';
    return;
  }
  // care about sign
  if(value<0) {
    value=-value;
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='-';
  }
  // write unsigned
  serial_write_uint((uint16_t) value);
}

// convenience: write unsigned long integer (32bit) 
void serial_write_ulint(uint32_t value) {
  // device busy
  if(g_writeln_st!=0) return;
  // eliminate singular case '0'
  if(value==0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0';
    return;
  }
  // figure number of digits
  uint32_t divisor=1;
  while(value>=divisor*10) {
    divisor=divisor*10;
    if(divisor>=1000000000UL) break; // max 4G >> divisor max 1G
  }  
  // print digits  
  while(divisor>0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0'+value/divisor;
    value=value % divisor;
    divisor = divisor/10;
  }  
}    

// convenience: write long integer (32bit) 
void serial_write_lint(int32_t value) {
  // device busy
  if(g_writeln_st!=0) return;
  // eliminate singular case '0'
  if(value==0) {
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='0';
    return;
  }
  // care about sign
  if(value<0) {
    value=-value;
    if(g_writeln_pos>=IOBUFFLEN) return;
    g_writeln_buf[g_writeln_pos++]='-';
  }
  // write unsigned
  serial_write_ulint((uint32_t) value);
}

// linebuffer
char g_readln_buf[IOBUFFLEN+2];   // actual buffer (+1 for terminating 0; +1 for editing syntactic sugar)
unsigned char g_readln_pos=0;     // currrent position to write to
volatile char g_readln_st=0;      // state: 0<>reading; 1<>line complete; 2<>processing line

// interrupt service routine to receive a character
ISR(USART0_RXC_vect) {
  unsigned char data=USART0_RXDATAL;             // reading the data clears the interrupt flag
  if(g_readln_st!=0) return;                     // drop data until recent line has been processed
  if(data=='\r')  return;                        // ingore control characters
  if(data=='\n') { g_readln_st=1; return;}       // sense line complete
  if(g_readln_pos>=IOBUFFLEN) return;            // drop data if buffer is full		     
  g_readln_buf[g_readln_pos++]=data;             // record data
}

// flush line buffer
void serial_readln_flush(void) {
  g_readln_pos=0;    
  g_readln_st=0; 
}  

// access recent line (call once to get line, call again to indicate that the line was processed)
char* serial_readln(void) {
  if(g_readln_st==0) return NULL;
  if(g_readln_st==1) {
    g_readln_buf[g_readln_pos]='\0';
    g_readln_st=2;
    return g_readln_buf;   
  }
  serial_readln_flush();
  return NULL;
}


/*
*************************************************************************
load/save config
*************************************************************************
*/

// checksum for nvm
int16_t g_nvmchk=0;

// read from EEPROM (addresses 0 to 127, atmega4808 maps EERPROM at 0x1400)
int16_t nvm_read(uint16_t addr) {
  int16_t data =  *(int16_t *)(((addr<<1) & 0xFF) | 0x1400);
  return data;
}

// write to EEPROM (interrupts must be disabled; atmega4808 maps EERPROM at 0x1400)
void nvm_write(uint16_t addr, int16_t data) {
  g_nvmchk+=data;
  *(int16_t *)(((addr<<1) & 0xFF) | 0x1400)=data; 
  //_PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
  CPU_CCP = CCP_SPM_gc;          
  NVMCTRL.CTRLA=NVMCTRL_CMD_PAGEERASEWRITE_gc;
  while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm);
}  

// read parameters from eeprom
void conf_load(void){
  // figure eeprom checksum
  char i;
  g_nvmchk=0;
  for(i=0; i< PARCNT; i++) g_nvmchk+=nvm_read(i);
  if(g_nvmchk != nvm_read(PARCNT)) {
    g_error|=ERR_CONF;
    return;
  }  
  // load 
  p_smaxcur  = nvm_read( 0);
  p_phases   = nvm_read( 1);
  p_lopenms  = nvm_read( 2);
  p_lclosems = nvm_read( 3);
}

// write parameters to eeprom (signature for cli, return 1 on success)
int16_t conf_save(int16_t val){
  char i;
  // bail out
  if(!val)
    return 0;
  if(g_ccs_st>= B0)
    return 0;
  // save
  CPU_SREG &= ~CPU_I_bm;
  g_nvmchk=0;
  nvm_write( 0,p_smaxcur);
  nvm_write( 1,p_phases);
  nvm_write( 2,p_lopenms);
  nvm_write( 3,p_lclosems);
  nvm_write(PARCNT,g_nvmchk);
  CPU_SREG |= CPU_I_bm;
#ifdef DEBUG_NVM
  //dump eeprom
  serial_writeln_sync();
  for(i=0; i<PARCNT; i++) {
    serial_write_str("% nvm write ");
    serial_write_int(nvm_read(i));
    serial_write_eol();
  }  
  serial_writeln_async();
#endif
  // figure checksum
  g_nvmchk=0;
  for(i=0; i<PARCNT; i++) g_nvmchk+=nvm_read(i);
  if(g_nvmchk != nvm_read(PARCNT))
    return 0;
  return 1;
}

/*
*************************************************************************
digital gpio
- all other digital gpios (SSRs, signal relay, lock)
- state machine to operate lock via callback
- elementary function to operate gpios
*************************************************************************
*/


void dio_init(){
  // SSRs
  PORTA.DIRSET = (PIN6_bm | PIN5_bm | PIN4_bm);   // SSR1, SSR2 and SSR3 on PA6, PA5 and PA4
  PORTA.OUTCLR = (PIN6_bm | PIN5_bm | PIN4_bm);   // all SSRs default to off
  // signal relay
  PORTA.DIRSET = PIN3_bm;                         // signal relay on PA3
  PORTA.OUTSET = PIN3_bm;                         // inverted driver SET<>off
  // lock drive
  PORTA.DIRSET = PIN7_bm;                         // lock drive A on PA7 (red wire)
  PORTA.OUTSET = PIN7_bm;                         // defaults to on
  PORTC.DIRSET = PIN2_bm;                         // lock drive B on PC2 (white wire)
  PORTC.OUTSET = PIN2_bm;                         // defaults to on
  // lock state
  PORTD.DIRCLR = PIN2_bm;                         // lock state on PD2 (black wire)
}  


// lock state machine
typedef enum {
  open=0,     // lock is open
  closing=1,  // lock is driven to closed
  closed=2,   // lock is closed
  opening=3,  // lock is driven to open
  error=4     // lock is in error (stuck closed)
} lock_state_t;
  
// external state variable (open lock on power up)
lock_state_t g_lock_st=opening;

// button sense callback
void lock_cb(void) {
  // no lock present ~> simulate
  if((p_lclosems==0) || (p_lopenms==0) ) {	
    if(g_lock_st==opening)  g_lock_st=open;
    if(g_lock_st==closing)  g_lock_st=closed;
    return;
  }
  // overall state
  static lock_state_t recstate=error;
  static uint16_t ondue=0;
  static uint16_t offdue=0;
  static uint16_t chkdue=0;
  static uint16_t reper=1000;
  static unsigned char retry;
  static bool drive=false;
  // update lock reading while drive is off
  static char lock_contact=-1; // -1 <> invalid; 0 <> open; 1 <> closed;
  static bool lcrec=-1;
  static char lccnt=0;
  if(!drive) {
    char lcnow = ((PORTD.IN & PIN2_bm) == 0 ? 0 : 1); // (black wire)
    if(lcnow==lcrec) {
      lccnt++;
    } else{
      lccnt=0;
    }
    lcrec=lcnow;
    if(lccnt>=10) {
      lccnt=0;
      if(lock_contact!=lcnow) {
	lock_contact=lcnow;
#ifdef DEBUG_LOCK
        serial_writeln_sync();
        serial_write_str("% lock: contact=");
        serial_write_int(lock_contact);
        serial_write_eol();
        serial_writeln_async();
#endif
      }
    }
  } else {
    lcrec=-1;
    lock_contact=-1;
  }  
  // sense commands
  if(recstate!=g_lock_st) {
    if(g_lock_st==closing) {
      DBW_LOCK(serial_writeln("% lock: closing"));
      // schedule closing operation with retries
      retry=3;
      ondue=g_systicks;
      offdue=g_systicks+p_lclosems;
      chkdue=g_systicks+p_lclosems+500;
      reper= p_lclosems+500+2000;
      reper=1000*(reper/1000+1);
    }
    if(g_lock_st==opening) {
      DBW_LOCK(serial_writeln("% lock: opening"));
      // schedule opening operation with retries
      retry=10;
      ondue=g_systicks;
      offdue=g_systicks+p_lopenms;
      chkdue=g_systicks+p_lopenms+500;
      reper= p_lopenms+500+2000;
      reper=1000*(reper/1000+1);
    }
    recstate=g_lock_st;
  }
  // do close
  if(g_lock_st==closing) {
    // turn power on
    if(TRIGGER_SCHEDULE(ondue)) {
      DBW_LOCK(serial_writeln("% lock: drive close"));
      ondue+=reper;
      PORTA.OUTSET = PIN7_bm; // terminal lock A 12V (red wire)			       
      PORTC.OUTCLR = PIN2_bm; // terminal lock B 0V (white wire)
      drive=true;
    }  
    // turn power off
    if(TRIGGER_SCHEDULE(offdue)) {
      DBW_LOCK(serial_writeln("% lock: drive off"));
      offdue+=reper;
      PORTA.OUTSET = PIN7_bm; // terminal lock A 12V (red wire)			       
      PORTC.OUTSET = PIN2_bm; // terminal lock B 12V (white wire)
      drive=false;
    }  
    // test lock
    if(TRIGGER_SCHEDULE(chkdue)) {
      DBW_LOCK(serial_writeln("% lock: check contact"));
      chkdue+=reper;
      if(lock_contact==1) {
	g_lock_st=closed;
	DBW_LOCK(serial_writeln("% lock: closed"));
      } else {
	--retry;
	if(retry==0) {
	  g_lock_st=opening; // try to open the lock again
	  g_error |= ERR_LOCK;
 	  DBW_LOCK(serial_writeln("% lock: error (failed closing)"));
	}  
      }
    }
  }
  // do open
  if(g_lock_st==opening) {
    // turn power on
    if(TRIGGER_SCHEDULE(ondue)) {
      DBW_LOCK(serial_writeln("% lock: drive open"));
      ondue+=reper;
      PORTA.OUTCLR = PIN7_bm; // terminal lock A 0V  (red wire)			       
      PORTC.OUTSET = PIN2_bm; // terminal lock B 12V (white wire)
      drive=true;
    }  
    // turn power off
    if(TRIGGER_SCHEDULE(offdue)) {
      DBW_LOCK(serial_writeln("% lock: drive off"));
      offdue+=reper;
      PORTA.OUTSET = PIN7_bm; // terminal lock A 12V (red wire)			       
      PORTC.OUTSET = PIN2_bm; // terminal lock B 12V (white wire)
      drive=false;
    }  
    // test lock
    if(TRIGGER_SCHEDULE(chkdue)) {
      DBW_LOCK(serial_writeln("% lock: check contact"));
      chkdue+=reper;
      if(lock_contact==0) {
	g_lock_st=open;
	DBW_LOCK(serial_writeln("% lock: open"));
      } else {
	--retry;
	if(retry==0) {
	  g_lock_st=error;
	  g_error |= ERR_LOCK;
 	  DBW_LOCK(serial_writeln("% lock: error (failed opening)"));
	} 
      }
    }
  }  
}

//cli wrapper
int16_t lock(int16_t val) {
  if(val && (g_lock_st != closed) && (g_lock_st != error)) g_lock_st=closing;
  if(!val && (g_lock_st != open) && (g_lock_st != error)) g_lock_st=opening;
  return val;
}  

// operate SSRs with decimal encoded parameter, e.g "123" all on
int16_t ssr(int16_t val) {
  // turn all off
  if(val==0) {
    PORTA.OUTCLR = (PIN6_bm | PIN5_bm | PIN4_bm);
    return val;
  };
  // turn enabled on i.e. decode parameter
  int dphases=val;
  unsigned char setporta=0x00;
  while(dphases>0) {
    int lsd=dphases % 10;
    switch(lsd) {
    case 1: setporta |= PIN6_bm; break;
    case 2: setporta |= PIN5_bm; break;
    case 3: setporta |= PIN4_bm; break;
    default: break;
    }
    dphases = dphases /10;
  }
  PORTA.OUTSET = setporta;
  PORTA.OUTCLR = (PIN6_bm | PIN5_bm | PIN4_bm) & (~setporta);
  return val;
}  

// operate signal relay (inverted driver) (signature for cli wrapper)
int16_t sigrel(int16_t val) {
  if(val) PORTA.OUTCLR = PIN3_bm;
  else PORTA.OUTSET = PIN3_bm;
  return val;
}  


/*
*************************************************************************
synchronous analog reading with ADC0
- internal temperature sensor [status: not functional]
- contact pilot CP on PD1
- proximity pilot PP pn PD0
- periodic update by main loop callback
*************************************************************************
*/

// global variables for recent readings
int16_t g_temp=0;        // degree Celsius
int16_t g_ppilot=-1;     // max current of cable in 100mA (-1 for invalid reading)
int16_t g_cpilot=-1;     // status in Volt 12,9,6 (-1 for invalid reading)
int16_t g_cpilot_dt=-1;  // status in Volt 1 (-1 for invalid reading)
int16_t g_pilots=0;      // enable periodic pilot reading      

// initialise vref and pins
void adc_init(void) {
  VREF.CTRLA |= VREF_ADC0REFSEL_1V1_gc;         // provide 1.1V vref for temperature sensor
  VREF.CTRLB |= VREF_ADC0REFEN_bm;              // permanently enable vref
  PORTD.DIRCLR = PIN0_bm;
  PORTD.PIN0CTRL &= ~PORT_ISC_gm;
  PORTD.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital buffers and interrugt generation
  PORTD.PIN0CTRL &= ~PORT_PULLUPEN_bm;          // make sure, there is no pullup (default anyway)
  PORTD.DIRCLR = PIN1_bm;
  PORTD.PIN1CTRL &= ~PORT_ISC_gm;
  PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital buffers and interrugt generation
  PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm;          // make sure, there is no pullup (default anyway)
}

// read temperature  [not functional]
bool adc_temp(void) {
  // dont run if ADC0 is busy
  if(g_adc0_bsy) return false;
  g_adc0_bsy=true;
  // set up adc v1: by the books
  /*
  ADC0.CTRLA=0x0;                        // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC16_gc;      // take 16 samples 
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV16_gc;      // 10Mhz vs div16 >> 625KHz >> vs 13clocks per sample >> about 0.3ms conversion
  ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;    // internal VREF aka 1.1V
  ADC0.CTRLC |= ADC_SAMPCAP_bm;          // reduced sampling capacity (atmel docs for temperature reading)
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY64_gc;    // delay to take first sample in ADC_CLK  >> 0.1ms 
  ADC0.CTRLD |= ADC_ASDV_bm;             // variable delay between samples (0-15 ADC_CLK)
  ADC0.SAMPCTRL=31;                      // time to charge capacitor 0.05ms
  ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; // set mux on internal temperature sensor
  ADC0.INTCTRL = 0x00;                    // no interrupts
  */
  // set up adc v2: playing along
  ADC0.CTRLA=0x0;                        // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC1_gc;       // take only one sample
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV8_gc;       // 10Mhz vs div8 >> 1.25MHz
  ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;    // internal VREF aka 1.1V
  ADC0.CTRLC |= ADC_SAMPCAP_bm;          // reduced sampling capacity
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY64_gc;    // delay to take first sample in ADC_CLK
  ADC0.SAMPCTRL=31;                      // clocks to charge capacitor
  ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; // set mux on internal temperature sensor
  ADC0.INTCTRL = 0x00;                    // no interrupts
  // run conversion
  ADC0.CTRLA |= ADC_ENABLE_bm;
  ADC0.COMMAND |= ADC_STCONV_bm;
  while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  uint16_t temp = ADC0.RES;
  //temp>>= 4;                            // compensate X accumulated samples
  // use factory calibration (see atmel docs)
  uint32_t ctemp = temp-SIGROW.TEMPSENSE1;
  ctemp*=SIGROW.TEMPSENSE0;
  ctemp+=0x80;
  ctemp>>=8;
  ctemp-=273;
  // record result
  g_temp=ctemp;
  // disable adc
  ADC0.CTRLA &= ~ADC_ENABLE_bm;
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;     // just to be sure ... ref aka 3.3V
  // done
  g_adc0_bsy=false;
#ifdef DEBUG_ADC
  serial_writeln_sync();
  serial_write_str("% temp ");
  serial_write_int(ctemp);
  serial_write('@');
  serial_write_int(temp);
  serial_write_eol();
  serial_writeln_async();
#endif  
  return true;
}

// read pilots (takes about 1.5ms since we wait for PWM cycle)
bool adc_pilots(void) {
  // dont run if ADC0 is busy
  if(g_adc0_bsy) return false;
  g_adc0_bsy=true;
  // set up adc
  ADC0.CTRLA=0x0;                        // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC4_gc;       // take 4 samples 
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV8_gc;       // 10Mhz vs div8 (1.25MHz) vs 13clocks per conversion >> 0.04ms for 4 conversions
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;    // 3.3 VDD reference
  ADC0.CTRLC |= ADC_SAMPCAP_bm;          // reduced sampling capacity (recommended for high VREF))
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY16_gc;    // delay to take first sample in ADC_CLK  >> 0.01ms 
  ADC0.CTRLD |= ADC_ASDV_bm;             // variable delay between samples (0-15 ADC_CLK)
  ADC0.SAMPCTRL=0;                       // no extra time to charge capacitor
  ADC0.INTCTRL = 0x00;                   // no interrupts
  ADC0.CTRLA |= ADC_ENABLE_bm;
  // run conversion contact pilot CP
  ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;      // select CP on PD1
  while(TCA0.SINGLE.CNT < 250);          // wait for dualslope PWM to be low
  while(TCA0.SINGLE.CNT > 250);          // wait for dualslope PWM to just become be high for level reading
  ADC0.COMMAND = ADC_STCONV_bm;
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  int16_t cp=ADC0.RES;
  cp >>= 2;  
  // convert to CP value
  int16_t cpv;
  if(cp > 980)  cpv=12;                          //  12V +/- tolerance (tolerances taken from Thurnherr original source)
  else if((cp >  860) && (cp < 915)) cpv=9;      //   9V +/- tolerance
  else if((cp >  720) && (cp < 800)) cpv=6;      //   6V +/- tolerance
  else if((cp > 1024) &&  (cp <  1))  cpv=3;     //   3V +/- tolerance [not implementet]
  else cpv=-1;                                   // invalid reading
  // filter
  static int16_t cpnxt=0;
  static char cpcnt=0;
  if(cpv!=g_cpilot) {
    if(cpv!=cpnxt) {
      cpnxt=cpv;
      cpcnt=0;
    } else {
      ++cpcnt;
      if(cpcnt==5)
	g_cpilot=cpv;
    }
  }
  while(TCA0.SINGLE.CNT < 4750);          // wait for middle of dualslope PWM to be low for diod test
  ADC0.COMMAND = ADC_STCONV_bm;
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  int16_t dt=ADC0.RES;
  dt >>= 2;  
  // convert to CP value
  int16_t dtv;
  if((dt > 25) && (dt < 95))   dtv=1;          // 0.6V +/- tolerance
  else dtv=-1;                                 // invalid reading
  // filter
  static int16_t dtnxt=0;
  static char dtcnt=0;
  if(dtv!=g_cpilot_dt) {
    if(dtv!=dtnxt) {
      dtnxt=dtv;
      dtcnt=0;
    } else {
      ++dtcnt;
      if(dtcnt==5)
	g_cpilot_dt=dtv;
    }
  }
  // run conversion proximity pilot PP
  ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;      // select PP on PD0
  ADC0.COMMAND = ADC_STCONV_bm;
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  int16_t pp=ADC0.RES;
  pp >>= 2;
  // convert PP value
  int16_t ppv=ADC0.RES;
  if(pp>980) ppv = 130;                         // unconnected, max 13A
  else if((pp > 260) && (pp < 480)) ppv=200;    // 680R: max 20A
  else if((pp > 150) && (pp < 220)) ppv=320;    // 220R: Max Capacity 32A
  else if((pp > 75) && (pp < 120))  ppv=633;    // 100R: Max Capacity 63A
  else ppv=-1;                                  // invalid reading
  // filter
  static int16_t ppnxt=0;
  static char ppcnt=0;
  if(ppv!=g_ppilot) {
    if(ppv!=ppnxt) {
      ppnxt=ppv;
      ppcnt=0;
    } else {
      ++ppcnt;
      if(ppcnt==5)
	g_ppilot=ppv;
    }
  }
  // disable adc
  ADC0.CTRLA &= ~ADC_ENABLE_bm;
  g_adc0_bsy=false;
  // done
#ifdef DEBUG_ADC
  static char cnt=0;
  cnt++;
  if(cnt>100) {
    cnt=0;
    serial_writeln_sync();
    serial_write_str("% pilot update ");
    serial_write_str(" cp: ");
    serial_write_int(cpv);
    serial_write('@');
    serial_write_int(cp);
    serial_write_str("; dp: ");
    serial_write_int(dtv);
    serial_write('@');
    serial_write_int(dt);
    serial_write_str("; pp: ");
    serial_write_int(ppv);
    serial_write('@');
    serial_write_int(pp);
    serial_write_eol();
    serial_writeln_async();
  }  
#endif  
  return true;
}

// callback for periodic reading (10ms period)
void pilots_cb(void) {
  static const uint16_t period=10;
  static uint16_t duetime=0;
  if(TRIGGER_SCHEDULE(duetime)) {
     if(!g_pilots) {
      duetime+=period;
      return;
    }
    if(adc_pilots()) duetime +=period;
  }
}

// callback for periodic reading (5000ms period)
void temp_cb(void) {
  static const uint16_t period=5000;
  static uint16_t duetime=0;
  if(TRIGGER_SCHEDULE(duetime)){
    if(!g_pilots) {
      duetime+=period;
      return;
    }
    if(adc_temp()) duetime +=period;
  }  
}

// cli wrapper to enable periodic reading
int16_t pilots(int16_t val)  {
  if(val==g_pilots) return val;
  g_cpilot=-1;
  g_cpilot_dt=-1;
  g_ppilot=-1;
  g_pilots=val;
  return val;
}

/*
*************************************************************************
asynchronous rms current sampling
- use ADC0 for conversion
- use a flag to have the systime 10KHz ISR trigger conversion 
- collect samples via RESRDY ISR in global buffer
- process record when complete
*************************************************************************
*/

// prepare current tranformer pins PD5,PD6 and PD7 for adc input
void rms_init(void) {
  PORTD.DIRCLR = PIN5_bm;
  PORTD.PIN5CTRL &= ~PORT_ISC_gm;
  PORTD.PIN5CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital buffers and interrugt generation
  PORTD.PIN5CTRL &= ~PORT_PULLUPEN_bm;          // make sure, there is no pullup (default anyway)
  PORTD.DIRCLR = PIN6_bm;
  PORTD.PIN6CTRL &= ~PORT_ISC_gm;
  PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
  PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
  PORTD.DIRCLR = PIN7_bm;
  PORTD.PIN7CTRL &= ~PORT_ISC_gm;
  PORTD.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;
  PORTD.PIN7CTRL &= ~PORT_PULLUPEN_bm;
}


// global vars
int16_t g_rms1=-1;
int16_t g_rms2=-1;
int16_t g_rms3=-1;
int16_t g_cur1=-1;
int16_t g_cur2=-1;
int16_t g_cur3=-1;


// global state
typedef enum {
  idle=0,
  sampling=1,
  processing=2
} rms_state_t;
volatile rms_state_t g_rms_st=idle;       // state: 0<>idle; 1<>sampling; 2<> processing data
#define RMS_CNT 1000                      // number of samples to record
int16_t g_rms_buf[RMS_CNT];               // actual buffer
int g_rms_pos=0;                          // write pointer
int g_rms_phase=0;                        // phase currently measured/processed
uint16_t g_rms_start=0;                   // time stamp: sampling start
uint16_t g_rms_stop=0;                    // time stamp: sampling stop
int16_t g_rms=0;                          // periodically trigger updates for all phases


// start taking samples on specified phase 1,2 or 3
// - return true on success
// - advance state from idle to sampling
// - 10kHz interrupr service routine will trigger individual sample
bool rms_start(char phase) {
  //insist in idle
  if(g_rms_st!=idle) return false;
  // adc mutex
  if(g_adc0_bsy) return false;
  g_adc0_bsy=true;
  // set up adc
  ADC0.CTRLA=0x0;                         // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC1_gc;        // no accumlation of samples
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV8_gc;        // 10Mhz vs div8 vs 13clocks per sample >>> about 100KHz max samp. freq.
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;     // vdd rek aka 3.3V
  ADC0.CTRLC |= ADC_SAMPCAP_bm;           // reduced sampling capacity
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY128_gc;    // delay to take first sample in ADC_CLK cycles
  ADC0.SAMPCTRL=10;                       // take 10 ADC_CLK cycles to charge capacitor >> down to appx 50KHz max samp. freq.
  // selcet input pin
  switch(phase) {
  case 1:
    ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
    break;
  case 2:
    ADC0.MUXPOS = ADC_MUXPOS_AIN6_gc;
    break;
  case 3:
    ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
    break;
  default:
    g_adc0_bsy=false;
    return false; // ignore out of range ohase
  }
  g_rms_phase=phase;
  // enable interrupt
  ADC0.INTCTRL = ADC_RESRDY_bm;             // interrupt on completion
  // enable adc
  ADC0.CTRLA |= ADC_ENABLE_bm;
  // advance state
  g_rms_pos=0;
  g_rms_start=g_systicks;
  g_rms_st=sampling;
  // done
  return true;
}
  

// interrupt service routine to collect samples
// -- advances state from sampling to processing
ISR(ADC0_RESRDY_vect) {
  int data = ADC0.RES;           // this will also clear the interrupt flag
  if(g_rms_st!=sampling) return; // insist in state
  g_rms_buf[g_rms_pos++]=data;   // do record
  if(g_rms_pos>=RMS_CNT) {       // sense record complete
    g_rms_st=processing;         // advance state
    g_rms_stop=g_systicks;
  }
}


// parameters rms integer algorithm incl DT1
//#define RMS_DT1
#define RMS_K 8           // filter gain in bits, i.e. K=2**8=256  
#define RMS_V1 2          // scaling sum of squares in bits, i.e., V1=2**2=4
#define RMS_V2 10         // scaling sum of squares in bits, i.e., V2=2**10=1024
/*
#define RMS_K 4           // filter gain in bits, i.e. K=2**8=256  
#define RMS_V1 2          // scaling sum of squares in bits, i.e., V1=2**2=4
#define RMS_V2 6          // scaling sum of squares in bits, i.e., V2=2**10=1024
*/
#define RMS_NORM  ((2*RMS_V1+RMS_V2)/2 - RMS_K)  // normalise by "*Sqrt(V1V1V2)/K"


// rms record processing
// -- advances state from processing to idle
// -- process in slices
void rms_process(void) {
  // insist in state
  if(g_rms_st!=processing) return;
  // process in slices: processing state
  static int spos=-1;
  static int rmsZero=512;
  static unsigned long int sum=0;
  static uint16_t rms=0;
  static uint16_t cur=0;
#ifdef RMS_DT1  
  static int rmsSpre=0;
  static long int rmsFpre=0;
#endif  
  // do slice -1: figure zero by mean
  if(spos<0) {
    sum=0;
    for(spos=0;spos<RMS_CNT;++spos)
      sum+= g_rms_buf[spos];
    rmsZero=(sum+(RMS_CNT>>1))/RMS_CNT;
    // initialse rms
    sum=0;
    spos=0;
#ifdef RMS_DT1  
    rmsSpre=0;
    rmsFpre=0;
#endif    
    return;
  }
  // do slices 0,100,200, ... RMS_CNT: process 100 values
  if(spos<RMS_CNT) {
    int send=spos+100;
    if(send>RMS_CNT) send=RMS_CNT;    
    // plain RMS: sum squares
#ifndef RMS_DT1      
    for(;spos<send;++spos) {
      int rmsS = g_rms_buf[spos]-rmsZero;
      if(rmsS<0) rmsS=-rmsS;
      unsigned long int rmsSQR = ((unsigned long int) rmsS) * ((unsigned long int) rmsS);
      sum += rmsSQR;
    }
#else    
    // RMS incl. DT1 (see above for commented version)
    for(;spos<send;++spos) {
      int rmsS = g_rms_buf[spos]-rmsZero;
      long int rmsF=  rmsFpre +   ( ((long int) (rmsS - rmsSpre)) << RMS_K );
      rmsF=  rmsF - ( rmsF>>RMS_K );  
      long int rmsFpV1 = rmsF >> RMS_V1;
      if(rmsFpV1<0) rmsFpV1=-rmsFpV1;
      unsigned long int rmsFsqrpV = ((unsigned long int) rmsFpV1) * ((unsigned long int)  rmsFpV1) >> RMS_V2;
      sum += rmsFsqrpV;
      rmsSpre=rmsS;
      rmsFpre=rmsF;
    }
#endif    
    return;
  }
  // slice RMS_CNT: take root
  if(spos==RMS_CNT) {
    // take mean-root
    rms=sqrt((sum+RMS_CNT/2)/RMS_CNT)+0.5;
#ifdef RMS_DT1    
    rms >>= -RMS_NORM;
#endif    
    spos++;
    return;
  }  
  // slice RMS_CNT: finilise result
  if(spos==RMS_CNT+1) {
    // normalise mV/100mA    
    rms = ((uint32_t) rms * 3300 + 511)/1023;
    cur = ((uint32_t) rms * 3300 *100 + 511)/(1023UL*95UL); //prelim calibration 1mV * 0.95 <> 100mA
    // store to global param
    switch(g_rms_phase) {  
    case 1:
      g_rms1=rms;
      g_cur1=cur;
      break;
    case 2:
      g_rms2=rms;
      g_cur2=cur;
      break;
    case 3:
      g_rms3=rms;
      g_cur3=cur;
      break;
    default:
      break;
    }
    // back to idle
    g_rms_st=idle;
    g_adc0_bsy=false;
    spos=-1;
    // report
#ifdef DEBUG_RMS
    serial_writeln_sync();
    serial_write_str("% rms1=");
    serial_write_uint(g_rms1);
    serial_write_str("mV; rms2=");
    serial_write_uint(g_rms2);
    serial_write_str("mV; rms3=");
    serial_write_uint(g_rms3);
    serial_write_str("mV;");
    serial_write_eol();
    serial_writeln_async();
#endif
  }
}


// periodic update call-back
void rms_cb(void) {
  static const unsigned int period=1000; 
  static unsigned int duetime=5;
  static int nphase=1;
  // anything to process?
  rms_process();
  // figure schedule
  if(TRIGGER_SCHEDULE(duetime)){
    // disabled >> try later 
    if(!g_rms) {
      duetime+=period;
      return;
    }
    // trigger measurement
    if(!rms_start(nphase)) return;
    // schedule next phase
    ++nphase;
    if(nphase>3) nphase=1;
    duetime+= period;
  } 
}


// cli wrapper to enable periodic rms reading
int16_t rms(int16_t val)  {
  if(val==g_rms)
    return val;
  g_rms1=-1;
  g_rms2=-1;
  g_rms3=-1;
  g_cur1=-1;
  g_cur2=-1;
  g_cur3=-1;
  g_rms=val;
  return val;
}


// cli wrapper for single measurement  (set to 1 to start update) 
int16_t rms1_start(int16_t val) {
  if(val!=1) return 0;
  if(!rms_start(1)) return 0;
  return 1;
}
int16_t rms2_start(int16_t val) {
  if(val!=1) return 0;
  if(!rms_start(2)) return 0;
  return 1;
}
int16_t rms3_start(int16_t val) {
  if(val!=1) return 0;
  if(!rms_start(3)) return 0;
  return 1;
}


// development scratch for processing rms record (signature for cli wrapper)
int16_t rms_dump(int16_t val) {
  if(val!=1) return 0;
  // check buffer
  if((g_rms_pos!=RMS_CNT) || (g_rms_st!=idle)) {
    serial_writeln_sync();
    serial_writeln("% rms record incomplete");
    serial_writeln_async();
    return 0;
  }
  // report buffer
  serial_writeln("% rms record dump");
  // stage 1: figure average
  int rmsZero = 512;
  unsigned long int sum = 0;
  int pos=0;
  for(;pos<RMS_CNT;++pos) 
    sum+=g_rms_buf[pos];
  rmsZero=(sum + (RMS_CNT>>1)) /RMS_CNT;
  // stage 2: run DT1+ SumOfSqares
  int rmsSpre = g_rms_buf[0]-rmsZero;
  long int rmsFpre = 0;
  sum=0;
  pos=0;
  for(;pos<RMS_CNT;++pos) {
    // feed input
    int rmsS = g_rms_buf[pos]-rmsZero;
    // DT1 type filter to cancel DC offset    
    // f(i+1) = (K-1)/K f(i)  +  (K-1) (s(i+1) - s(i))   
    long int rmsF=  rmsFpre +   ( ((long int) (rmsS - rmsSpre)) << RMS_K );
    rmsF=  rmsF - ( rmsF>>RMS_K );  
    // take squares and normalise
    long int rmsFpV1 = rmsF >> RMS_V1;
    if(rmsFpV1<0) rmsFpV1=-rmsFpV1;
    unsigned long int rmsFsqrpV = ((unsigned long int) rmsFpV1) * ((unsigned long int)  rmsFpV1) >> RMS_V2;
    // sum all up
    sum += rmsFsqrpV;
    // update recents
    rmsSpre=rmsS;
    rmsFpre=rmsF;
    // report
    serial_writeln_sync();
    serial_write_str("% rms");
    serial_write_int(g_rms_phase);
    serial_write_str(" ");
    serial_write_int(rmsS);
    serial_write_str(" [F=");
    serial_write_lint(rmsF);
    serial_write_str("; Fsqr=");
    serial_write_ulint(rmsFsqrpV);
    serial_write_str("; Sum=");
    serial_write_ulint(sum);
    serial_write_str("]");
    serial_write_eol(); 
    serial_writeln_async();
  }
  // take root
  uint16_t rms=sqrt(sum/RMS_CNT)+0.5;
  rms >>= -RMS_NORM;
  // convert to mV (note: int rounding)
  rms = ((uint32_t) rms * 3300 + 511)/1023;
  // summary
  serial_writeln_sync();
  serial_write_str("% rms ");
  serial_write_uint(rms);
  serial_write_str("mV [mean #");
  serial_write_uint(rmsZero);
  serial_write_str("; cnt #");
  serial_write_int(RMS_CNT);
  serial_write_str("; sample time ");
  serial_write_int(g_rms_stop-g_rms_start);
  serial_write_str("ms]");
  serial_write_eol(); 
  serial_writeln_async();
  return 1;
}
  
/*
*************************************************************************
PWM output for CP driven by TCA on PA2
- 1KHz dual slope PWM
*************************************************************************
*/

// current pwm in 100mA
int16_t g_cpcurrent=0;

// initialse counter TCA
void cppwm_init() {
  PORTA.DIRSET = PIN2_bm;                              // configure digital output
  PORTA.OUTSET = PIN2_bm;                              // set to high as fallback, when PWM is disabled
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;           // rout TCA to PORTA (default anyway)
  TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV1_gc;      // set clock to 10MHz, 100ns per tick
  TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_DSBOTTOM_gc;  // set to dual slope mode
  TCA0.SINGLE.PER    = 5000;                           // 1ms = 2 * period * 100ns >> period= 5000;
  TCA0.SINGLE.CMP2   = 500;                            // use cmp2 for PWM of PA2, default duty cycle 10% for testing  
  TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP2EN_bm;          // disable output    
  TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;           // start timer (timer runs allways == relevant for CP reading)
}

// set duty cycle [100mA]:  "max_current[A]=dutycycle[%] * 0.6[A]" for a range of 10%-85%
int16_t cpcurrent(int16_t cpcur) {
  // spcecial case: 0<>off<>fallback dio<>high<>12V
  if(cpcur==0) {
    TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP2EN_bm;
    g_cpcurrent=0;
    return 0;
  }
  // pwm is set fine --- dont mess
  if(g_cpcurrent==cpcur)
    return g_cpcurrent;
  // figure duticycle [permil]
  int dc= (10*cpcur+3)/6;
  // refuse too low setting
  if(dc<100) return g_cpcurrent;
  // limit too high setting
  if(dc>850) dc=850;
  // set pwm
  TCA0.SINGLE.CMP2BUF = (5000/1000) * dc;
  TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
  // record max current
  g_cpcurrent= (dc*6 +5)/10;
  return g_cpcurrent;
}  



/*
*************************************************************************
system time driven by xmega TCB
- provide 1ms systicks as unsigned int (16bit) --- for convenient binary arithmatics
- provide 1ms systime with 30000 rollover (half a minute) --- for external synchronisation 
- run short real-time tasks in 100us interupt handler
*************************************************************************
*/

// global systicks/time in ms
uint16_t g_systicks=0;
int16_t g_systime=0;

// local systicks/time for ISR
uint16_t g_systicks_isr=0;
int16_t g_systime_isr=0;

// setup RTC to produce 100us interrupt
void systime_init(void) {
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc;    // run at 10.000.000Hz
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;       // periodically run from 0 up to CCMP excl 
  TCB0.INTCTRL = TCB_CAPT_bm;            // trigger interrupt for each period
  TCB0.CCMP = 1000;                      // 1000 <==> 10.000Hz
  TCB0.CTRLA |= TCB_ENABLE_bm;           // start timer now
}  
    
// interrupt service routine 100us
ISR(TCB0_INT_vect) {
  // fast 0.1ms tasks come here --- start RMS adc conversion on 10KHz
  if(g_rms_st==sampling) {
    ADC0.COMMAND |= ADC_STCONV_bm;
  }  
  // divide by 10 for 1ms periode
  static unsigned char tcb_10s=0;
  ++tcb_10s;
  if(tcb_10s<10) {
    TCB0.INTFLAGS |= TCB_CAPT_bm;        // clear interupt flag
    return;
  }  
  tcb_10s=0;
  // slow 1ms tasks come here
  ++g_systicks_isr;
  ++g_systime_isr;
  if(g_systime_isr==30000) g_systime_isr=0;
  TCB0.INTFLAGS |= TCB_CAPT_bm;          // clear interupt flag
}  

// update global systime/ticks
void systime_cb(void) {
  while(TCB0.CNT>950);       // wait until at least 50 counts until next interrupt
  g_systicks=g_systicks_isr; // 16 cycles
  g_systime=g_systime_isr;   // 16 cycles
}

// set systime (signature for cli)
int16_t systime_set(int16_t val) {
  while(TCB0.CNT>950);       // wait until at least 50 counts until next interrupt
  g_systime_isr=val;
  g_systime=val;
  return val;
}  

// globals for cycle time monitor
uint16_t g_cyclemin=0xffff;
uint16_t g_cyclemax=0;
uint16_t g_cyclerec=0;
uint16_t g_cycleskip=true;

// monitor cycle time
void monitor_cb() {
  int16_t ctime=g_systicks-g_cyclerec;
  g_cyclerec=g_systicks;
  if(!g_cycleskip) {
    if(ctime<g_cyclemin) g_cyclemin=ctime;
    if(ctime>g_cyclemax) g_cyclemax=ctime;
  }
  g_cycleskip=false;
}   

// report cycle time
void serial_write_monitor(void) {
  serial_write_str("cycle time min/max ");
  serial_write_uint(g_cyclemin);
  serial_write('/');
  serial_write_uint(g_cyclemax);
  serial_write_eol();
  g_cyclemin=0xffff;
  g_cyclemax=0;
  g_cycleskip=true;
}    
 

/*
*************************************************************************
*************************************************************************
*************************************************************************

command line parser
- format is "<PAR>?\r\n" for get and "<PAR>=<VAL>\r\n" for set
- both, get and set reply with <PAR>=<VAL>\r\n", or "fail"
- syntactic sugar "<PAR>!" to set the parameter to 1 (e.g. trigger some action)
- syntactic sugar "<PAR>~" to set the parameter to 0 
- the parser converts the string argument to functions/addresses to call/read/write
- dedicated special commands are implemented on top (eg plain '?' for "list all")

*************************************************************************
*************************************************************************
*************************************************************************
*/

// set/get function types
typedef int16_t (*parsetfnct_t)(int16_t);
typedef int16_t (*pargetfnct_t)(void);


// name to token relation
typedef struct {
    char* parstr;              // parameter name    
    const int16_t* pargetaddr; // address in memory for reading 
    int16_t* parsetaddr;       // address in memory for writing
    parsetfnct_t parsetfnct;   // function for writing
} partable_t;


// name to token table (NULL-terminated --- should be in PROGMEM)
const partable_t partable[]={
  // normal operation
  {"ver",     &g_version,   NULL,        NULL},            // g_version can be read from memory
  {"blinks",  &g_blinks,    &g_blinks,   NULL},            // g_blinks can be read/written from/to memory
  {"time",    &g_systime,   NULL,        &systime_set},    // g_systime has explicit setter
  {"temp",    &g_temp,      NULL,        NULL},            // g_temp can be read from memory
  {"error",   &g_error,     NULL,        NULL},            // g_error can be read from memory
  {"ccss",    &g_ccss_cli,  NULL,        NULL},            // g_ccs_st can be read from memory
  {"cmaxcur", &g_ppilot,    NULL,        NULL},            // get cable max current (same as PPilot)
  {"amaxcur", &g_cpcurrent, NULL,        NULL},            // get actual max current as set on PWM
  {"cur1",    &g_cur1,      NULL,        NULL},            // read rms measurement phase L1
  {"cur2",    &g_cur2,      NULL,        NULL},            // read rms measurement phase L2
  {"cur3",    &g_cur3,      NULL,        NULL},            // read rms measurement phase L3
  // configure
#ifdef MODE_CONFIGURE  
  {"save",    NULL,         NULL,        &conf_save},      // save parameters to eeprom
  {"smaxcur", &p_smaxcur,   &p_smaxcur,  NULL},            // max mains current
  {"phases",  &p_phases,    &p_phases,   NULL},            // enabled phases (decimal encoding)  
  {"lopenms", &p_lopenms,   &p_lopenms,  NULL},            // time to open lock
  {"lclosems",&p_lclosems,  &p_lclosems, NULL},            // time to close lock
#endif  
  // develop
#ifdef MODE_DEVELOP  
  {"cpcur",   &g_cpcurrent, NULL,        &cpcurrent},      // set pwm via setter, get from memory
  {"rms",     &g_rms,       NULL,        &rms},            // g_rms enables/disables rms periodic updates
  {"pilots",  &g_pilots,    NULL,        &pilots},         // g_pilots enables/disables periodic pilot updates
  {"cpilot",  &g_cpilot,    NULL,        NULL},            // cp pilot read-back
  {"cpdtest", &g_cpilot_dt, NULL,        NULL},            // cp pilot read-back on low slope, aka "diodtest"
  {"ppilot",  &g_ppilot,    NULL,        NULL},            // pp pilot read-back (in 100mA)
  {"rms1",    &g_rms1,      NULL,        &rms1_start},     // "rms1!" to trigger a single rms measurement on phase L1
  {"rms2",    &g_rms2,      NULL,        &rms2_start},     // "rms2!" to trigger a single rms measurement on phase L2
  {"rms3",    &g_rms3,      NULL,        &rms3_start},     // "rms3!" to trigger a single rms measurement on phase L3
  {"rmsdmp",  NULL,         NULL,        &rms_dump},       // "rmsdmp!" dumps recent rms record for debugging
  {"sigrel",  NULL,         NULL,        &sigrel},         // "sigrel!"/"sigrel~" to operate pilot signal relay
  {"lock",    NULL,         NULL,        &lock},           // "lock!"/"lock~" to operate lock
  {"ssr",     NULL,         NULL,        &ssr},            // "ssr(123) to operate all SSRs
  {"reset",   NULL,         NULL,        &reset},          // softreset "reset!"
#endif  
  // end of table
  {NULL, NULL, NULL, NULL},            
};


// list all parameters to serial line (tuned to set sync writeln)
void serial_write_parlist(void) {
  // iterate table
  const partable_t* ptab=partable;
  while(ptab->parstr!=NULL) {
    serial_write_str(ptab->parstr);
    if(ptab->pargetaddr) {
      serial_write('=');
      serial_write_int(*ptab->pargetaddr);
    }
    serial_write_eol();
    ++ptab;
  }
}  


// parse command line, take appropriate action, return true on success
bool parse(char* line){
  DBW_CLI(serial_writeln("% parse A"));
  // find first occurrence of '?', '=', '!' or '~',
  char* pos=line;
  while(*pos!=0) {
    if(*pos=='?') break;
    if(*pos=='=') break;
    if(*pos=='!') break;
    if(*pos=='~') break;
    ++pos;
  }
  if(*pos==0) return false;
  DBW_CLI(serial_writeln("% parse B"));
  // save seperator, terminate parname, proceed pos to value/0
  char sep=*pos;
  *(pos++)=0;
  // figure table entry
  const partable_t* ptab=partable;
  while(ptab->parstr!=NULL) {
    if(!strcmp(ptab->parstr,line)) break;
    ++ptab;
  }
  if(ptab->parstr==NULL) return false;
  DBW_CLI(serial_writeln("% parse C"));
  // synthatic sugare "!" for "=1"
  if(sep=='!') {
    *(pos)='1';
    *(pos+1)='\0';
    sep='=';
  }    
  // synthatic sugare "~" for "=0"
  if(sep=='~') {
    *(pos)='0';
    *(pos+1)='\0';
    sep='=';
  }    
  // if its a set, figure the value pos is pointing to
  int16_t val=0;
  if(sep=='=') {
    DBW_CLI(serial_write_str("% parse N:"));
    DBW_CLI(serial_writeln(pos));
    if(*pos=='\0') return false;
    int sign =1;
    if(*pos=='-') {
      sign=-1;
      ++pos;
    }  
    while(*pos!='\0') {
      if((*pos<'0') || (*pos> '9')) break;
      if(val > 3200) return false;
      val=10*val+(*pos-'0');
      ++pos;
    }
    val=val*sign;
  }
  DBW_CLI(serial_writeln("% parse D"));
  // if its a get ..
  if(sep=='?') {
    if(ptab->pargetaddr) val=*ptab->pargetaddr;
    else return false;
  }
  DBW_CLI(serial_writeln("% parse E"));
  // if its a set ..
  if(sep=='=') {
    if(ptab->parsetfnct) val=(*ptab->parsetfnct)(val);
    else
      if(ptab->parsetaddr) *ptab->parsetaddr=val;
      else
	return false;
  }
  DBW_CLI(serial_writeln("% parse F"));
  // reply
  serial_write_str(line);
  serial_write('=');
  serial_write_int(val);
  serial_write_eol();
  return true;
}


// overall command line interface (aka std parameter set/get + some special commands)
void cmdline(char* ln) {
  // special command: list all status
  if(*ln=='?') {
    serial_writeln_sync();
    serial_writeln("[[[");
    serial_write_parlist();
    serial_writeln("===");
    serial_write_monitor();
    serial_write_str("systime: ");
    serial_write_uint(g_systime);
    serial_write_eol();
    serial_write_str("lock state: ");
    serial_write_uint(g_lock_st);
    serial_write_eol();
    serial_write_str("charger state: ");
    serial_write_uint(g_ccs_st);
    serial_write_eol();
    serial_writeln("]]]");
    serial_writeln_async();
    g_cycleskip=true;
    return;
  }
  // std get/set parameter 
#ifdef DEGUGGING
  serial_writeln_sync();   // make sure that we can reply
#endif
  bool ok=parse(ln);
  if(!ok) serial_writeln("fail");
  // complete any ongoing transmission (only when there is debug output)
#ifdef DEGUGGING
  serial_writeln_async();  // make sure that reply is transmitted
#endif
}  


/*
*************************************************************************
*************************************************************************
*************************************************************************

main loop

*************************************************************************
*************************************************************************
*************************************************************************
*/


// ev charging state machine
void ccs_cb(void) {
  static uint16_t toutA;
  static uint16_t toutB;
  static uint16_t toutW;
  static uint16_t toutP;
  static uint16_t maxcur;
  static uint16_t phases;
  
  // state OFF0: all off: wait for confirmed open lock
  if(g_ccs_st==OFF0) {
    ssr(0);
    rms(0);
    cpcurrent(0);
    pilots(0);
    sigrel(0);
    g_blinks=BLINKS_HEART;
    lock(0);
    if(g_lock_st==open) {
      DBW_CCS(serial_writeln("% ccs state: OFF0 -> OFF1"));
      g_ccs_st=OFF1;
    }  
  }  
  // state OFF1: wait for button press --> state A
  if(g_ccs_st==OFF1) {
    if(g_button) {
      DBW_CCS(serial_writeln("% OFF1 -> A0"));
      g_ccs_st=A0;
    }
    g_button=false;
  }  
  // state A0 (idle): enable CP at 100% for const +12V
  if(g_ccs_st==A0) {
    ssr(0);
    rms(0);
    pilots(1);
    cpcurrent(0);
    sigrel(1);
    g_blinks=BLINKS_RELAX;
    toutA=g_systicks+10000;
    DBW_CCS(serial_writeln("% A0 -> A1"));
    g_ccs_st=A1;
  }  
  // state A1 (idle): validate EV present, i.e., CP to falls to 9V and appropriate pp -> state B (or timeout)
  if(g_ccs_st==A1) {
    if((g_cpilot==9) && (g_ppilot>0)) {
      DBW_CCS(serial_writeln("% A1 -> B0"));
      g_ccs_st=B0;
    }
    if(TRIGGER_SCHEDULE(toutA)) {
      DBW_CCS(serial_writeln("% A1 -> ERROR"));
      g_ccs_st=ERR0;
    }
  }  
  // state B0 (EV present): lock
  if(g_ccs_st==B0) {
    ssr(0);
    rms(0);
    pilots(1);
    sigrel(1);
    lock(1);
    g_blinks=BLINKS_RELAX;
    maxcur=p_smaxcur;
    phases=p_phases;
    if(maxcur>g_ppilot) maxcur=g_ppilot;
    if(g_lock_st==closed) {
      if((maxcur>=60) && (phases!=0)) {
	toutB=g_systicks+10000;
        DBW_CCS(serial_writeln("% B0 -> B1"));    
        g_ccs_st=B1;
      } else {
        DBW_CCS(serial_writeln("% B0 -> W0"));    
        g_ccs_st=W0;
      }
    }
  }
  // state B1 (EV present and locked): wait for vehicle ready to charge, i.e., CP falls to 6V --> state C (or timeout --> state OFF)
  if(g_ccs_st==B1) {
    cpcurrent(maxcur);
    if(g_cpilot==6) {
      DBW_CCS(serial_writeln("% B1 -> C0/1"));
      g_ccs_st=C0;
    }
    if(TRIGGER_SCHEDULE(toutB)) {
      DBW_CCS(serial_writeln("% B1 -> ERR0"));
      g_ccs_st=ERR0;
    }
  }
  // state C0 (EV about to charge): sanity checks
  if(g_ccs_st==C0) {
    if( (maxcur<60) || (phases==0) || (g_lock_st!=closed) ) {
      DBW_CCS(serial_writeln("% C0 -> ERR"));
      g_ccs_st=ERR0;
    } else {
      DBW_CCS(serial_writeln("% C0 -> C1"));
      g_ccs_st=C1;
    }
  }
  // state C1 (EV charging)
  if(g_ccs_st==C1) {
    cpcurrent(maxcur);
    sigrel(1);
    ssr(phases);
    rms(1);
    pilots(1);
    g_blinks=BLINKS_ON;
    DBW_CCS(serial_writeln("% C1 -> C2"));
    g_ccs_st=C2;
  }  
  // state C2 (EV charging): sense vehicle can't charge no more, i.e., CP raises to 9V --> state B (or button press --> state OFF)
  if(g_ccs_st==C2) {
    if(g_cpilot==9) {
      DBW_CCS(serial_writeln("% C2 -> B0"));
      g_ccs_st=B0;
    }
    if((g_cpilot!=9) && (g_cpilot!=6)) {
      DBW_CCS(serial_writeln("% C2 -> OFF0 (pilot)"));
      g_ccs_st=OFF0;
    }
    if(g_button) {
      DBW_CCS(serial_writeln("% C2 -> OFF0 (button)"));
      g_ccs_st=OFF0;
    }
    // update configuration
    if((p_phases!=phases) || (p_smaxcur<60) || (g_ppilot<60)) {
      DBW_CCS(serial_writeln("% C2 -> P0"));
      g_ccs_st=P0;
    } else { 
      maxcur=p_smaxcur;
      phases=p_phases;
      if(maxcur>g_ppilot) maxcur=g_ppilot;
      cpcurrent(maxcur);
      ssr(phases);
    }
  }
  // state P0 (EV charging): set timer to pause charging in 10sec
  if(g_ccs_st==P0) {
    cpcurrent(0);
    toutP=g_systicks+10000;
    DBW_CCS(serial_writeln("% P0 -> P1"));
    g_ccs_st=P1;
  }
  // state P1 (EV charging): pause charging
  if(g_ccs_st==P1) {
    if(TRIGGER_SCHEDULE(toutP)) {
      ssr(0);  
      sigrel(0);
      rms(0);
      toutW=g_systicks+10000;
      DBW_CCS(serial_writeln("% P1 -> W0"));
      g_ccs_st=W0;
    }
  }    
  // state W0 (EV idle): idle for 10secs
  if(g_ccs_st==W0) {
    ssr(0);  
    sigrel(0);
    g_blinks=BLINKS_RELAX;
    if(g_button) {
      DBW_CCS(serial_writeln("% W0 -> OFF0"));
      g_ccs_st=OFF0;
    }  
    if(TRIGGER_SCHEDULE(toutW)) {
      DBW_CCS(serial_writeln("% W0 -> W1"));
      g_ccs_st=W1;
    }
  }    
  // state W1 (EV idle): wait for power allocation
  if(g_ccs_st==W1) {    
    if(g_button) {
      DBW_CCS(serial_writeln("% W1 -> OFF0"));
      g_ccs_st=OFF0;
    }  
    maxcur=p_smaxcur;
    if(maxcur>g_ppilot) maxcur=g_ppilot;
    if((maxcur>=60) && (p_phases!=0)) {
      DBW_CCS(serial_writeln("% W1 -> A0"));
      g_ccs_st=A0;
    }
  }
  // error state (protocol time outs)
  if(g_ccs_st==ERR0) {
    g_error|=ERR_CCS;
  }  
  // update cli vartaint of state
  g_ccss_cli=g_ccs_st;
}  



int main(){

  // initialize all hardware
  clock_init();
  serial_init();
  ledbutton_init();
  dio_init();
  adc_init();
  rms_init();
  cppwm_init();
  systime_init();

  // start interrupts now
  CPU_SREG |= CPU_I_bm;

  // read configuratiom
  conf_load();
  
  // run forever (target for less than 10ms cycle time)
  while(1){

    // aux callbacks for status updates
    systime_cb();     // update systime and systicks
    monitor_cb();     // monitor cycle time
    led_blinks_cb();  // flash led
    lock_cb();        // operate lock via external state g_lock_state 
    button_cb();      // sense positive edge on button via g_button
    rms_cb();         // keep updating asynchronous current meassurement (max 5ms)
    pilots_cb();      // keep updating synchronous analog reading of pilots (max 1.5ms)
    temp_cb();        // keep updating synchronous analog reading of temperature

    // error handler
    if(g_error!=0) {
      static uint16_t recdue=0; 
      static uint16_t recerr=0x0000; 
      // turn all off
      sigrel(0);
      ssr(0);
      rms(0);
      pilots(0);
      lock(0);
      // signal
      g_blinks=BLINKS_ERR;
      // lock error: if lock open we recover from lock error in 10sec
      if((g_lock_st==open) && (g_error & ERR_LOCK) && (recerr==0x0000)) {
        recerr=ERR_LOCK;
	recdue=g_systicks+10000;
      }
      // CCS error only: recover in 30secs
      if((g_error==ERR_CCS) && (recerr==0x0000)) {
	recerr=ERR_CCS;
	recdue=g_systicks+30000;
      }	
      // recover
      if(recerr!=0x0000) {
	if(TRIGGER_SCHEDULE(recdue)) {
	  g_error&= ~recerr;
          if(g_error==0x0000)
	    g_ccs_st=OFF0;
	  recerr=0x0000;
	}
      }		
    }

    // run ccs charging state machine
    if(g_error==0) {
      ccs_cb();
    }  
          
    // command line interface      
    char* ln = serial_readln();
    if(ln) {
      cmdline(ln);
      serial_readln_flush();
    }
    
  } // loop forever

  // never get there
  return(0);
}




