/*
*************************************************************************
*************************************************************************
*************************************************************************

AGCCS Ctrl22C

- target platform agccs board (set revision as runtime parameter)
- compiles with avr-gcc, verified avr-libc 2.0. and gcc 7.3
- although this code is a complete re-write from scratch, we did 
  (a) conceptually benefit from inspecting the SmartEVSE firmware by 
  Michael Stegen (https://github.com/SmartEVSE and more speifically the code 
  in "SmartEVSE-2/SmartEVSE2.X/EVSE.c")
  (b) build upon the first-installation code by Pascal Thurnherr's regarding
  port configuration (https://github.com/dreadnomad/FGCCS-Ctrl22)
- this software is distributed by the same license terms as the projects 
  (a) and (b) above, except that the copyright is with Thomas Moor. 


*************************************************************************
*************************************************************************
*************************************************************************

Copyright Thomas Moor 2020-2021

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

// firmware revision 2021-04-25

// firmware version for OTA
#define CTRL22C_VERSION 13  // XY reads vX.Y, i.e., one digit for major and minor, resp.


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


// have version as parameter to facilitate CLI read out
const int16_t g_version=CTRL22C_VERSION;   

/*
*************************************************************************
*************************************************************************
*************************************************************************

calibration and configuration (defaults can be saved/loaded to/from eeprom)

- calibration parameters for AC current measurement
- configuration data for max load
- configuration data for enabled phases
- configuration data for optional lock
 
*************************************************************************
*************************************************************************
*************************************************************************
*/

#define PARCNT 9          // number of parameters
int16_t p_boardrev=12;    // board rev 1.2
int16_t p_imaxcur=160;    // installed max current in [100mA]
int16_t p_iphases=123;    // enstalled phases
int16_t p_lclsms=100;     // ms to close lock (set to 0 for no lock)
int16_t p_lopnms=100;     // ms to open lock  (set to 0 for no lock)
int16_t p_caldmp=0;       // set to 1 to turn on calibration output
int16_t p_cala=19019;     // calibration parameter a
int16_t p_calb=0;         // calibration parameter b
int16_t p_ccsdmp=0;       // set to 1 to turn on ccs state progress



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
#define MODE_INSTALL                         // enable mode for code/hardware tests
#define MODE_CONFIGURE                       // enable mode for calibration/configuration 


// serial debug should write synchronous
#define DEBUG_WRITE(c)  {serial_writeln_sync(); c; serial_writeln_async();}

// debug command line interface
//#define DEBUG_CLI
#ifdef DEBUG_CLI
#define DBW_CLI(c)  DEBUG_WRITE(c)
#else
#define DBW_CLI(c)
#endif

// debug analog reading (pilots, temp, vcc)
//#define DEBUG_ADC
#ifdef DEBUG_ADC
#define DBW_ADC(c)  DEBUG_WRITE(c)
#else
#define DBW_ADC(c)
#endif

// debug nvm access
//#define DEBUG_NVM
#ifdef DEBUG_NVM
#define DBW_NVM(c)  DEBUG_WRITE(c)
#else
#define DBW_NVM(c)
#endif


// debug lock
//#define DEBUG_LOCK
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

// convenience: return true if duetime has expired (uint16 ticks, max 32767ms schedule))
#define TRIGGER_SCHEDULE(duetime)  (   \
   ((g_systicks >= duetime) &&  (g_systicks-duetime < 0x8000)) ||  \
   ((g_systicks < duetime) &&  (duetime-g_systicks > 0x8000)) )

// forward declaration of ccs charging state; see ccs_cb()
typedef enum {
  OFF0=0,OFF1=1,OFF2=2,OFF3,OFF9=9,  // OFF (nominal: wait for operator)
  A0=10,A1=11,                       // A   (nominal: wait for EV detect)
  B0=20,B1=21,                       // B   (nominal: wait for EV ready to charge)
  C0=30,C1=31,C2=32,C3=33,           // C   (nominal: do charge)
  P0=40,P1=41,                       // P   (initiate pause)
  W0=50,W6=56,W7=57,W8=58,W9=59,     // W   (wait for change of power allocation, resume with A)
  ERR0=90
} ccs_state_t;
ccs_state_t g_ccs_st=OFF0;

// cli veriant of ccs state (how do we properly/safly cast an enum to an int16_t?)
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
  while(1);
  // never get here
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



// extra flash patterns
#define BLINKS_ON    21
#define BLINKS_OFF    0
#define BLINKS_HEART  1
#define BLINKS_RELAX 22
#define BLINKS_ERR   20

// blink pattern overwrite by serial line
int g_blinks=-1;
int g_xblinks=-1;


// flash my led (callback in main loop)
void led_blinks_cb(void) {
  // set blinks to indicate ccss state
  int blinks=BLINKS_RELAX;
  if(g_ccs_st<OFF9) blinks=BLINKS_HEART;
  if((g_ccs_st>=C0) && (g_ccs_st<P0)) blinks=BLINKS_ON;
  // overwrite by error
  if(g_error!=0) blinks=BLINKS_ERR;
  // overwrite by serial, normal priority 
  if(g_blinks>0) blinks=g_blinks;
  // overwrite by serial, high priority 
  if(g_xblinks>0) blinks=g_xblinks;
  // const on
  if(blinks==BLINKS_ON) {
    led_on();
    return;
  }
  // const off
  if(blinks==BLINKS_OFF) {
    led_off();
    return;
  }
  // sync on 2sec with systime (30sec rollover) 
  int16_t led_time=g_systime % 2000;
  int16_t led_cycle;
  // fast cycle, range 1-20, incl BLINKS_ERR
  if(blinks<=20) {
    led_cycle = led_time / 100 +1;
    if(led_cycle > blinks) {
      led_off();
      return;
    }  
    if(led_time % 100 <50)
      led_on();
    else
      led_off();
  }
  // slow cycle, range 1-4, for BLINK_RELAX
  if(blinks==BLINKS_RELAX) {
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

// button pressed (incl debounce, user must reset)
bool g_button=false;

// button sense callback
void button_cb(void) {
  static bool dbtn=false;
  static bool filter=false;
  static char cnt=0;
  bool btn = PORTD.IN & PIN4_bm;
  if(btn==filter) {
    ++cnt;
  } else {
    filter=btn;
    cnt=0;
  }
  if(cnt==50) {
    if(btn && (!dbtn)) g_button=true;
    dbtn=btn;
    cnt=0;
  }
}
  


/*
*************************************************************************
serial line on uart0
-- write buffered line asynchronously 
-- read buffered line asynchronously 
-- effectively support a line-by-line serial protocol
*************************************************************************
*/

// baudrate formula from Atmel documentation
#define USART_BAUD_RATE(BAUD_RATE) (uint16_t) ( (F_CPU * 64.0) / (16.0 * BAUD_RATE) + 0.5 ) 

// component: init serial line on usart0 qith pins PA0 (TX) and PA1 (RX)
void serial_init(void) {
  USART0.BAUD  = USART_BAUD_RATE(115200);                                // set baud rate
  USART0.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;         // 8 data bits, one stop bit, no parity
  USART0.CTRLA |= USART_RXCIE_bm;                                        // enable RX interupt
  PORTA.DIRSET = PIN0_bm;                                                // set TX pin
  PORTA.DIRCLR = PIN1_bm;                                                // set RX pin
  USART0.CTRLB |= (USART_TXEN_bm | USART_RXEN_bm);                       // endable UART
}

// linebuffer
#define IOBUFFLEN  80              // buffer size
char g_writeln_buf[IOBUFFLEN+3];   // actual buffer (+1 for terminating 0, +2 for "\r\n")
unsigned char g_writeln_pos=0;     // currrent position to write to
volatile char g_writeln_st=0;      // state: 0<>writing to buffer; 1<>sending via uart
bool g_writeln_sync=false;         // true<>synchronous mode; false<>asynchronous mode

// interrupt service routine to send characters from 0-terminated buffer
ISR(USART0_DRE_vect) {
  char data=g_writeln_buf[g_writeln_pos++];          // get data from buffer
  if((data=='\0') || (g_writeln_pos>=IOBUFFLEN+3)) { // sense end of string
    USART0.CTRLA  &= ~USART_DREIE_bm;                // disable DRE interupt
    g_writeln_pos=0;
    g_writeln_st=0;
    return;
  }  
  USART0_TXDATAL=data;                               // write byte to data register
}

// true when buffer is ready
bool serial_write_ready(void) {
  return g_writeln_st==0;
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

// convenience: write 0-terminated string from progmem
void serial_write_pgmstr(const char* pstr) {
  if(g_writeln_st!=0) return;
  char c;
  while(((c=pgm_read_byte(pstr))!='\0') && (g_writeln_pos<IOBUFFLEN)) {
    g_writeln_buf[g_writeln_pos++]=c;
    pstr++;
  }
}

// convenience: define string constant in progmem and write
#define serial_write_pstr(cstr) serial_write_pgmstr(PSTR(cstr))

// convenience: write 0-terminated string as line
void serial_write_ln(const char* str){
  serial_write_str(str);
  serial_write_eol();
}

// convenience: write 0-terminated string from progmen as line
void serial_write_pgmln(const char* pstr){
  serial_write_pgmstr(pstr);
  serial_write_eol();
}

// convenience: define string in progmem and write a line
#define serial_write_pln(cstr) serial_write_pgmln(PSTR(cstr))

// convenience: forward to tab
void serial_write_tab(int pos) {
  if(g_writeln_st!=0) return;
  while((g_writeln_pos<pos) && (g_writeln_pos<IOBUFFLEN))
    g_writeln_buf[g_writeln_pos++]=' ';
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
int16_t nvm_read(uint16_t waddr) {
  int16_t data =  *(int16_t *)(((waddr<<1) & 0xFF) | 0x1400);
  return data;
}

// write to EEPROM (interrupts must be disabled; atmega4808 maps EERPROM at 0x1400)
void nvm_write(uint16_t waddr, int16_t data) {
  g_nvmchk+=data;
  *(int16_t *)(((waddr<<1) & 0xFF) | 0x1400)=data; 
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
  p_boardrev = nvm_read( 0);
  p_imaxcur  = nvm_read( 1);
  p_iphases  = nvm_read( 2);
  p_lopnms   = nvm_read( 3);
  p_lclsms   = nvm_read( 4);
  p_caldmp   = nvm_read( 5);
  p_cala     = nvm_read( 6);
  p_calb     = nvm_read( 7);
  p_ccsdmp   = nvm_read( 8);
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
  nvm_write( 0,p_boardrev);
  nvm_write( 1,p_imaxcur);
  nvm_write( 2,p_iphases);
  nvm_write( 3,p_lopnms);
  nvm_write( 4,p_lclsms);
  nvm_write( 5,p_caldmp);
  nvm_write( 6,p_cala);
  nvm_write( 7,p_calb);
  nvm_write( 8,p_ccsdmp);
  nvm_write(PARCNT,g_nvmchk);
  CPU_SREG |= CPU_I_bm;
#ifdef DEBUG_NVM
  //dump eeprom
  serial_writeln_sync();
  for(i=0; i<PARCNT; i++) {
    serial_write_pstr("% nvm write ");
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
digital io
- all other digital gpios (SSRs, signal relay, lock, brownout)
- state machine to operate lock via callback
- elementary function to operate the gpios
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
  // brown out
  PORTF.DIRCLR = PIN0_bm;                         // brown out signal is of PF0)
}


// lock drive configuation
typedef enum {
  ldopen=0,   // drive towards open
  ldclose=1,  // drive towards close
  ldoff=2     // drive off
} ldrive_t;
static ldrive_t g_ldrive=ldoff;

// lock primitves
void ldrive(ldrive_t cmd) {
  switch(cmd) {
  case ldopen:    
    PORTA.OUTCLR = PIN7_bm; // terminal lock A 0V (red wire)			       
    PORTC.OUTSET = PIN2_bm; // terminal lock B 12V (white wire)
    break;
  case ldclose:  
    PORTA.OUTSET = PIN7_bm; // terminal lock A 12V (red wire)			       
    PORTC.OUTCLR = PIN2_bm; // terminal lock B 0V (white wire)
    break;
  case ldoff:
  default:
    PORTA.OUTSET = PIN7_bm; // terminal lock A 12V (red wire)			       
    PORTC.OUTSET = PIN2_bm; // terminal lock B 12V (white wire)
  }
  g_ldrive=cmd;
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
  if((p_lclsms==0) || (p_lopnms==0) ) {	
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
  // update lock reading while drive is off
  static char lock_contact=-1; // -1 <> invalid; 0 <> open; 1 <> closed;
  static bool filter=-1;
  static char cnt=0;
  if(g_ldrive==ldoff) {
    char lc = ((PORTD.IN & PIN2_bm) == 0 ? 0 : 1); // (black wire)
    if(lc==filter) {
      cnt++;
    } else{
      filter=lc;
      cnt=0;
    }
    if(cnt>=10) {
      cnt=0;
#ifdef DEBUG_LOCK
      if(lock_contact!=lc) {
        serial_writeln_sync();
        serial_write_pstr("% lock: contact=");
        serial_write_int(lc);
        serial_write_eol();
        serial_writeln_async();
      }	
#endif
      lock_contact=lc;
    }
  } else {
    filter=-1;
    cnt=0;
    lock_contact=-1;
  }  
  // sense commands
  if(recstate!=g_lock_st) {
    if(g_lock_st==closing) {
      DBW_LOCK(serial_write_pln("% lock: closing"));
      // schedule closing operation with retries
      retry=3;
      ondue=g_systicks;
      offdue=g_systicks+p_lclsms;
      chkdue=g_systicks+p_lclsms+500;
      reper= p_lclsms+500+2000;
      reper=1000*(reper/1000+1);
    }
    if(g_lock_st==opening) {
      DBW_LOCK(serial_write_pln("% lock: opening"));
      // schedule opening operation with retries
      retry=10;
      ondue=g_systicks;
      offdue=g_systicks+p_lopnms;
      chkdue=g_systicks+p_lopnms+500;
      reper= p_lopnms+500+2000;
      reper=1000*(reper/1000+1);
    }
    recstate=g_lock_st;
  }
  // do close
  if(g_lock_st==closing) {
    // turn power on
    if(TRIGGER_SCHEDULE(ondue)) {
      DBW_LOCK(serial_write_pln("% lock: drive close"));
      ondue+=reper;
      ldrive(ldclose);
    }  
    // turn power off
    if(TRIGGER_SCHEDULE(offdue)) {
      DBW_LOCK(serial_write_pln("% lock: drive off"));
      offdue+=reper;
      ldrive(ldoff);
    }  
    // test lock
    if(TRIGGER_SCHEDULE(chkdue)) {
      DBW_LOCK(serial_write_pln("% lock: check contact"));
      chkdue+=reper;
      if(lock_contact==1) {
	g_lock_st=closed;
	DBW_LOCK(serial_write_pln("% lock: closed"));
      } else {
	--retry;
	if(retry==0) {
	  g_lock_st=opening; // try to open the lock again
	  g_error |= ERR_LOCK;
 	  DBW_LOCK(serial_write_pln("% lock: error (failed closing)"));
	}  
      }
    }
  }
  // do open
  if(g_lock_st==opening) {
    // turn power on
    if(TRIGGER_SCHEDULE(ondue)) {
      DBW_LOCK(serial_write_pln("% lock: drive open"));
      ondue+=reper;
      ldrive(ldopen);
    }  
    // turn power off
    if(TRIGGER_SCHEDULE(offdue)) {
      DBW_LOCK(serial_write_pln("% lock: drive off"));
      offdue+=reper;
      ldrive(ldoff);
    }  
    // test lock
    if(TRIGGER_SCHEDULE(chkdue)) {
      DBW_LOCK(serial_write_pln("% lock: check contact"));
      chkdue+=reper;
      if(lock_contact==0) {
	g_lock_st=open;
	DBW_LOCK(serial_write_pln("% lock: open"));
      } else {
	--retry;
	if(retry==0) {
	  g_lock_st=error;
	  g_error |= ERR_LOCK;
 	  DBW_LOCK(serial_write_pln("% lock: error (failed opening)"));
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

// record currently enabled phases
int16_t g_ssrphases=0;
int16_t g_ssrphases_bin=0;

// operate SSRs with decimal encoded parameter, e.g "123" all on
int16_t ssr(int16_t val) {
  // record state
  // turn all off
  if(val==0) {
    PORTA.OUTCLR = (PIN6_bm | PIN5_bm | PIN4_bm);
    g_ssrphases=0;
    g_ssrphases_bin=0;
    return val;
  };
  // turn enabled on i.e. decode parameter
  g_ssrphases=val;
  g_ssrphases_bin=0;
  int dphases=val;
  unsigned char setporta=0x00;
  while(dphases>0) {
    int lsd=dphases % 10;
    switch(lsd) {
    case 1: {setporta |= PIN6_bm; g_ssrphases_bin|=1; } break;
    case 2: {setporta |= PIN5_bm; g_ssrphases_bin|=2; } break;
    case 3: {setporta |= PIN4_bm; g_ssrphases_bin|=3; } break;
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

// check brown out (open lock is our last action)
void bot_cb(void) {
  // no brown-out detection in early boards
  if(p_boardrev<1.2) return;
  // read brown-out detector
  bool bot = !(PORTF.IN & PIN0_bm);
  if(!bot) return;
  ldrive(ldopen);
  ssr(0);
  serial_writeln_sync();
  serial_write_pln("[bot]");
  while(1);
}  


/*
*************************************************************************
set up internal voltage reference
- we to avoid reconfiguring the reference, so there are two vialabe configurations
a) set to 2.5V reference:
   this is good for RMS measurement since it annihilates some of the VDD ripple;
   however, it is not so good for temeparure reading since we loose resolution
b) use 1.1V reference: 
   good for for temperature measurement; the RMS measurement will then use
   VDD as a reference
*************************************************************************
*/

#define VREF25 // chosee this for a)
//#define VREF11 // choose this for b)

void vref_init(void) {
#ifdef VREF25
  VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc;          // provide 2.5V to adc
  VREF.CTRLA |= VREF_AC0REFSEL_2V5_gc;           // provide 2.5V to analog compare
#endif
#ifdef VREF11  
  VREF.CTRLA |= VREF_ADC0REFSEL_1V1_gc;          // provide 1.5V to adc
  VREF.CTRLA |= VREF_AC0REFSEL_1V1_gc;           // provide 1.5V to analog compare
#endif
  VREF.CTRLB |= VREF_ADC0REFEN_bm;               // permanently enable vref
  AC0.CTRLA |= AC_ENABLE_bm;                     // enable analog compare
  AC0.DACREF =255;                               // set dacref to 255/256*vref
}




/*
*************************************************************************
synchronous analog reading with ADC0
- internal temperature sensor 
- contact pilot CP on PD1
- proximity pilot PP pn PD0
- periodic update by main loop callback
*************************************************************************
*/

// global variables for recent readings
int16_t g_temp=0;        // degree Celsius
int16_t g_vcc=0;         // mV
int16_t g_cpilot=-1;     // status in Volt 12,9,6 (-1 for invalid reading)
int16_t g_cpnslp=-1;     // status in Volt -12 (-1 for invalid reading)
int16_t g_ppilot=-1;     // max current of cable in 100mA (-1 for invalid reading)
int16_t g_pilots=0;      // enable periodic pilot reading


// global variables to filter readings
int16_t g_filter_p=-2;
int16_t g_filter_c=-2;
int16_t g_filter_d=-2;


// initialise vref and pins
void adc_init(void) {
  PORTD.DIRCLR = PIN0_bm;
  PORTD.PIN0CTRL &= ~PORT_ISC_gm;
  PORTD.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital buffers and interrugt generation
  PORTD.PIN0CTRL &= ~PORT_PULLUPEN_bm;          // make sure, there is no pullup (default anyway)
  PORTD.DIRCLR = PIN1_bm;
  PORTD.PIN1CTRL &= ~PORT_ISC_gm;
  PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital buffers and interrugt generation
  PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm;          // make sure, there is no pullup (default anyway)
}

// read temperature  
bool adc_temp(void) {
  // dont run if ADC0 is busy
  if(g_adc0_bsy) return false;
  g_adc0_bsy=true;
  // set up adc 
  ADC0.CTRLA=0x0;                        // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC16_gc;      // take 16 samples 
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV16_gc;      // 10Mhz vs div16 >> 625KHz >> vs 13clocks per sample >> about 0.3ms conversion
  ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;    // internal VREF (by the books 1.1V, see vref_init())
  ADC0.CTRLC |= ADC_SAMPCAP_bm;          // reduced sampling capacity (atmel docs for temperature reading)
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY64_gc;    // delay to take first sample in ADC_CLK  >> 0.1ms 
  ADC0.CTRLD |= ADC_ASDV_bm;             // variable delay between samples (0-15 ADC_CLK)
  ADC0.SAMPCTRL=31;                      // time to charge capacitor 0.05ms
  ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; // set mux on internal temperature sensor
  ADC0.INTCTRL = 0x00;                   // no interrupts
  // run conversion
  ADC0.CTRLA |= ADC_ENABLE_bm;
  ADC0.COMMAND |= ADC_STCONV_bm;
  while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  uint16_t temp = ADC0.RES;
  // compensate for 16 accumulated samples and/or non-standard ref aka different to 1.1V
#ifdef VREF11  
  temp= (temp+16/2)/16;                  // comp. 16 acc. samples
#else
#ifdef VREF25    
  temp= (16L*((uint32_t) temp)+(16*16*11/25)/2)/(16*16*11/25); // comp. 16 acc. samples and 2.5V ref
#else
  temp=0
#endif
#endif    
  // use factory calibration (see atmel docs)
  uint32_t ctemp = temp- ((int8_t) SIGROW.TEMPSENSE1);
  ctemp*=((uint8_t) SIGROW.TEMPSENSE0);
  ctemp=(ctemp+256/2)/256; 
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
  serial_write_pstr("% temp ");
  serial_write_int(ctemp);
  serial_write('@');
  serial_write_int(temp);
  serial_write_pstr(" (");
  serial_write_int(SIGROW.TEMPSENSE0);
  serial_write('/');
  serial_write_uint(SIGROW.TEMPSENSE1);
  serial_write(')');
  serial_write_eol();
  serial_writeln_async();
#endif  
  return true;
}

// read VCC
bool adc_vcc(void) {
  // dont run if ADC0 is busy
  if(g_adc0_bsy) return false;
  g_adc0_bsy=true;
  // set up adc 
  ADC0.CTRLA=0x0;                        // disable adc for re-config, 10bit resolution, all default
  ADC0.CTRLB= ADC_SAMPNUM_ACC16_gc;      // take 16 samples 
  ADC0.CTRLC=0x0;
  ADC0.CTRLC |= ADC_PRESC_DIV16_gc;      // 10Mhz vs div16 >> 625KHz >> vs 13clocks per sample >> about 0.3ms conversion
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;    // make VCC the reference
  ADC0.CTRLC |= ADC_SAMPCAP_bm;          // reduced sampling capacity
  ADC0.CTRLD=0x0;
  ADC0.CTRLD |= ADC_INITDLY_DLY64_gc;    // delay to take first sample in ADC_CLK  >> 0.1ms 
  ADC0.CTRLD |= ADC_ASDV_bm;             // variable delay between samples (0-15 ADC_CLK)
  ADC0.SAMPCTRL=31;                      // time to charge capacitor 0.05ms
  ADC0.MUXPOS = ADC_MUXPOS_DACREF_gc;    // set mux to measure internal bandgap (1.1V or 2.5V, see vref_init()
  ADC0.INTCTRL = 0x00;                   // no interrupts
  // run conversion
  ADC0.CTRLA |= ADC_ENABLE_bm;
  ADC0.COMMAND |= ADC_STCONV_bm;
  while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  uint16_t vcc = ADC0.RES;
  // convert by "RES = 1023*DACREF/VCC", i.e., VCC=1023*DACREF/RES" (also comp.for 16 acc. samples)
#ifdef VREF11  
  g_vcc= ( ((uint32_t) (1100.0*16*1023*255/256 +0.5)) + ((uint32_t) (vcc/2)) ) /vcc;
#else
#ifdef VREF25    
  g_vcc= ( ((uint32_t) (2500.0*16*1023*255/256L+0.5)) + ((uint32_t) (vcc/2)) ) /vcc;
#else
  g_vcc=0
#endif
#endif    
  // disable adc
  ADC0.CTRLA &= ~ADC_ENABLE_bm;
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;     // just to be sure ... ref aka 3.3V
  // done
  g_adc0_bsy=false;
#ifdef DEBUG_ADC
  serial_writeln_sync();
  serial_write_pstr("% vcc ");
  serial_write_int(g_vcc);
  serial_write('@');
  serial_write_int(vcc);
  serial_write_eol();
  serial_writeln_async();
#endif  
  return true;
}

// callback for periodic reading of temperature and VCC (10000ms period)
void env_cb(void) {
  static const uint16_t period=10000;
  static uint16_t duetime=0;
  static bool toggle=false;
  if(TRIGGER_SCHEDULE(duetime)){
    if(toggle) {
      if(adc_temp()) {duetime +=period; toggle=false;}
    } else {
      if(adc_vcc()) {duetime +=period; toggle=true;}
    }      
  }  
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
  // convert to CP value (changed R13 on board rev1.2)
  int16_t cpv;
  if(p_boardrev<12) {
    if(cp > 980)  cpv=12;                          //  12V +/- tolerance (tolerances taken from Thurnherr original source)
    else if((cp >  860) && (cp < 915)) cpv=9;      //   9V +/- tolerance
    else if((cp >  720) && (cp < 800)) cpv=6;      //   6V +/- tolerance
    else if((cp > 1024) &&  (cp <  1)) cpv=3;      //   3V +/- tolerance [not implementet >> invalid]
    else cpv=-1;                                   // invalid reading
  } else {  
    if(cp > 861)  cpv=12;                          //  12V +/- tolerance [11V..12V]
    else if((cp >  779) && (cp < 834)) cpv=9;      //   9V +/- tolerance [8V..10V]
    else if((cp >  697) && (cp < 752)) cpv=6;      //   6V +/- tolerance [5V..7V]
    else if((cp > 1024) &&  (cp <  1)) cpv=3;      //   3V +/- tolerance [not implementet >> invalid]
    else cpv=-1;                                   // invalid reading
  }  
  // filter
  static char ccnt;
  if(g_filter_c==-2) {
    ccnt=0;
    g_filter_c=-1;
  }  
  if(cpv==g_filter_c) {
    ccnt++;
  } else {
    g_filter_c=cpv;
    ccnt=0;
  }  
  if(ccnt==5) {    
    g_cpilot=cpv;
  }
  // run conversion contact pilot CP
  while(TCA0.SINGLE.CNT < 4750);          // wait for middle of dualslope PWM for the negative halfwave
  ADC0.COMMAND = ADC_STCONV_bm;
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  int16_t dt=ADC0.RES;
  dt >>= 2;  
  // convert to CP value (changed R13 on board rev1.2)
  int16_t dtv;
  if(p_boardrev<12) {
    if((dt > 25) && (dt < 95))   dtv=12;         // -12V +/- tolerance [taken from Thurnherr original source] 
    else dtv=-1;                                 // invalid reading
  } else {
    if((dt > 221) && (dt < 275)) dtv=12;         // -11.4V +/- tolerance [-12.4V ... -11.4V]
    else dtv=-1;                                 // invalid reading
  }
  // filter
  static char dcnt;
  if(g_filter_d==-2) {
    dcnt=0;
    g_filter_d=-1;
  }  
  if(dtv==g_filter_d) {
    dcnt++;
  } else {
    g_filter_d=dtv;
    dcnt=0;
  }  
  if(dcnt==5) {    
    g_cpnslp=dtv;
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
  else if((pp > 75) && (pp < 120))  ppv=630;    // 100R: Max Capacity 63A
  else ppv=-1;                                  // invalid reading 
  // filter
  static char pcnt;
  if(g_filter_p==-2) {
    pcnt=0;
    g_filter_p=-1;
  }  
  if(ppv==g_filter_p) {
    pcnt++;
  } else {
    g_filter_p=ppv;
    pcnt=0;
  }  
  if(pcnt==5) {    
    g_ppilot=ppv;
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
    serial_write_pstr("% pilot update: ");
    serial_write_pstr(" cp: ");
    serial_write_int(cpv);
    serial_write('@');
    serial_write_int(cp);
    serial_write_pstr("; dp: ");
    serial_write_int(dtv);
    serial_write('@');
    serial_write_int(dt);
    serial_write_pstr("; pp: ");
    serial_write_int(ppv);
    serial_write('@');
    serial_write_int(pp);
    serial_write_eol();
    serial_writeln_async();
  }  
#endif  
  return true;
}

// callback for periodic reading of pilots (16ms period)
void pilots_cb(void) {
  static const uint16_t period=16;
  static uint16_t duetime=0;
  // while inactive postpone
  if(!g_pilots) {
    if(TRIGGER_SCHEDULE(duetime)) duetime+=period;
    return;
  }
  // figure schedule
  if(TRIGGER_SCHEDULE(duetime)) 
    if(adc_pilots()) duetime +=period;
}

// cli wrapper to enable periodic reading of pilots
int16_t pilots(int16_t val)  {
  if(val==g_pilots) return val;
  g_cpilot=-1;
  g_cpnslp=-1;
  g_ppilot=-1;
  g_filter_c=-2;
  g_filter_d=-2;
  g_filter_p=-2;
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
int16_t g_sns1=-1;
int16_t g_sns2=-1;
int16_t g_sns3=-1;
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
#define RMS_CNT 1000                      // number of samples to record (must be multiple of 4)
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
#ifdef VREF25
  ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;     // select internal reference if set on 2.5V
#else
  ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;     // fallback to VDD as reference aka 3.3V
#endif  
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
    return false; // ignore out of range phase
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
  uint16_t sns=0;
  uint16_t cur=0;
  // do slice -1: figure zero by mean
  if(spos<0) {
    sum=0;
    for(spos=0;spos<RMS_CNT;++spos)
      sum+= g_rms_buf[spos];
    rmsZero=(sum+(RMS_CNT/2))/RMS_CNT;
    // initialse rms
    sum=0;
    spos=0;
    return;
  }
  // do slices 0,100,200, ... RMS_CNT: process 100 values
  if(spos<RMS_CNT) {
    int send=spos+100;
    if(send>RMS_CNT) send=RMS_CNT;    
    // sum the squares
    for(;spos<send;++spos) {
      int rmsS = g_rms_buf[spos]-rmsZero;
      if(rmsS<0) rmsS=-rmsS;
      unsigned long int rmsSQR = ((unsigned long int) rmsS) * ((unsigned long int) rmsS);
      sum += rmsSQR;
    }
    return;
  }
  // slice RMS_CNT: take root
  if(spos==RMS_CNT) {
    // provide  mean-root by 11bit (i.e. keep one extra bit) 
    rms=sqrt((sum+(RMS_CNT/4)/2)/(RMS_CNT/4))+0.5;
    spos++;
    return;
  }  
  // slice RMS_CNT: finilize result
  if(spos==RMS_CNT+1) {
    // normalise to mV (rms*Vref/4046)
#ifdef VREF25    
    sns = ((uint32_t) rms * 80078L + (1L<<15)) >> 16; // avoid division: 2500/4046 * 2^16=80078
#else
    sns = ((uint32_t) rms * 3300 + (1023*2)/2)/(1023*2); //  by the books ;-)
#endif
    // first calibration: 1mV * 0.95 <> 100mA ==> cal_A= (0.95*Vref/2046)*2^14=19019
    cur = ((uint32_t) rms * p_cala + (1L<<13)) >> 14;
    cur+= p_calb;
    // anihilate low-level noise and negative cal_B
    if(cur<10) cur=0;
    // store to global parameter
    switch(g_rms_phase) {  
    case 1:
      g_rms1=rms;
      g_sns1=sns;
      g_cur1=cur;
      break;
    case 2:
      g_rms2=rms;
      g_sns2=sns;
      g_cur2=cur;
      break;
    case 3:
      g_rms3=rms;
      g_sns3=sns;
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
    if(p_caldmp) {
      serial_writeln_sync();
      serial_write_pstr("% rms:");
      if(g_cur1>=0) {    
        serial_write_pstr(" L1: ");
        serial_write_uint(g_cur1);
        serial_write_pstr(" (");
        serial_write_uint(g_sns1);
        serial_write_pstr("mV@");
        serial_write_uint(g_rms1);
        serial_write_pstr(");");
      }
      if(g_cur2>=0) {    
        serial_write_pstr(" L2: ");
        serial_write_uint(g_cur2);
        serial_write_pstr(" (");
        serial_write_uint(g_sns2);
        serial_write_pstr("mV@");
        serial_write_uint(g_rms2);
        serial_write_pstr(");");
      }
      if(g_cur3>=0) {    
        serial_write_pstr(" L3: ");
        serial_write_uint(g_cur3);
        serial_write_pstr(" (");
        serial_write_uint(g_sns3);
        serial_write_pstr("mV@");
        serial_write_uint(g_rms3);
        serial_write_pstr(");");
      }
      serial_write_eol();
      serial_writeln_async();
    }   
  }
}


// periodic update call-back, 1024ms
void rms_cb(void) {
  static const unsigned int period=1024; 
  static unsigned int duetime=5;
  static int nphase=1;
  // abort stale reading, reschedule
  if(!g_rms) {
    g_rms_st=idle;
    if(TRIGGER_SCHEDULE(duetime)) duetime+=period;
    return;
  }      
  // process recored if any
  rms_process();
  // figure schedule
  if(TRIGGER_SCHEDULE(duetime)){
    if(rms_start(nphase)) {
      ++nphase;
      if(nphase>3) nphase=1;
      duetime+= period;
    }
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

// (includes test code with DT1 )
#define RMS_K 8           // filter gain in bits, i.e. K=2**8=256  
#define RMS_V1 2          // scaling sum of squares in bits, i.e., V1=2**2=4
#define RMS_V2 10         // scaling sum of squares in bits, i.e., V2=2**10=1024
#define RMS_NORM  ((2*RMS_V1+RMS_V2)/2 - RMS_K)  // normalise by "*Sqrt(V1V1V2)/K"

int16_t rms_dump(int16_t val) {
  if(val!=1) return 0;
  // check buffer
  if((g_rms_pos!=RMS_CNT) || (g_rms_st!=idle)) {
    serial_writeln_sync();
    serial_write_pln("% rms record incomplete");
    serial_writeln_async();
    return 0;
  }
  // report buffer
  serial_write_pln("% rms record dump");
  // stage 1: figure average
  int rmsZero = 512;
  unsigned long int sumF = 0;
  unsigned long int sumP = 0;
  int pos=0;
  for(;pos<RMS_CNT;++pos) 
    sumP+=g_rms_buf[pos];
  rmsZero=(sumP + (RMS_CNT>>1)) /RMS_CNT;
  // stage 2: run DT1+ SumOfSqares
  int rmsSpre = g_rms_buf[0]-rmsZero;
  long int rmsFpre = 0;
  sumP=0;
  sumF=0;
  pos=0;
  for(;pos<RMS_CNT;++pos) {
    // feed input
    int rmsS = g_rms_buf[pos]-rmsZero;
    // plain sum of squares
    unsigned long int rmsSQR = ((long int) rmsS) * ((long int) rmsS);
    sumP += rmsSQR;
    // DT1 type filter     
    // f(i+1) = (K-1)/K f(i)  +  (K-1) (s(i+1) - s(i))   
    long int rmsF=  rmsFpre +   ( ((long int) (rmsS - rmsSpre)) << RMS_K );
    rmsF=  rmsF - ( rmsF>>RMS_K );  
    // take squares and normalise
    long int rmsFpV1 = rmsF >> RMS_V1;
    if(rmsFpV1<0) rmsFpV1=-rmsFpV1;
    unsigned long int rmsFsqrpV = ((unsigned long int) rmsFpV1) * ((unsigned long int)  rmsFpV1) >> RMS_V2;
    // sum all up
    sumF += rmsFsqrpV;
    // update filter state
    rmsSpre=rmsS;
    rmsFpre=rmsF;
    // report
    serial_writeln_sync();
    serial_write_pstr("% rms");
    serial_write_int(g_rms_phase);
    serial_write_pstr(" ");
    serial_write_int(rmsS);
    serial_write_pstr(" [F=");
    serial_write_lint(rmsF);
    serial_write_pstr("; Fsqr=");
    serial_write_ulint(rmsFsqrpV);
    serial_write_pstr("; SumF=");
    serial_write_ulint(sumF);
    serial_write_pstr("; SumP=");
    serial_write_ulint(sumP);
    serial_write_pstr("]");
    serial_write_eol(); 
    serial_writeln_async();
  }
  // take root and normalise (plain squares)
  uint16_t rmsP=sqrt((sumP+(RMS_CNT/4)/2)/(RMS_CNT/4))+0.5; // mean root 11bit (i.e. keep one extra bit)
#ifdef VREF25    
    rmsP = ((uint32_t) rmsP * 2500 + 2047/2)/2047;
#else
    rmsP = ((uint32_t) rmsP * 3300 + 2047/2)/2047;
#endif
  // take root and normalise (filtered squares)
  uint16_t rmsF=sqrt(sumF/RMS_CNT)+0.5;
  rmsF >>= -RMS_NORM;
#ifdef VREF25    
  rmsF = ((uint32_t) rmsF * 2500 + 1023/2)/1023;
#else
  rmsF = ((uint32_t) rmsF * 3300 + 1023/2)/1023;
#endif  
  // summary
  serial_writeln_sync();
  serial_write_pstr("% rmsF: ");
  serial_write_uint(rmsF);
  serial_write_pstr("mV; rmsP: ");
  serial_write_uint(rmsP);
  serial_write_pstr("mV [mean #");
  serial_write_uint(rmsZero);
  serial_write_pstr("; cnt #");
  serial_write_int(RMS_CNT);
  serial_write_pstr("; sample time ");
  serial_write_int(g_rms_stop-g_rms_start);
  serial_write_pstr("ms]");
  serial_write_eol(); 
  serial_writeln_async();
  return 1;
}
  
/*
*************************************************************************
PWM output for CP driven by TCA on PA2
- 1kHz dual slope PWM
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
volatile uint16_t g_systicks_isr=0;
volatile int16_t g_systime_isr=0;

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
  if(g_systime_isr>=30000) g_systime_isr=0;
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
  serial_write_pstr("cycle time min/max ");
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

Playload: CCS state machine
-- this operates the lot

*************************************************************************
*************************************************************************
*************************************************************************
*/

// control parameters for remote power alocation
int16_t g_smaxcur=0;  // max current allowed by remote site 
int16_t g_sphases=0;  // phases allocated by remote site


// monitor CCS state od serial line
void write_ccss(const char* pstr) {
  if(!p_ccsdmp) return;
  serial_writeln_sync();
  serial_write_pgmstr(pstr);
  serial_write_eol();
  serial_writeln_async();
}

// convenience: use progmem
#define WRITE_CCSS(cstr) write_ccss(PSTR(cstr))

// set to initial state
void ccs_init() {
  // run without remote, set to installation default
  g_smaxcur=p_imaxcur;
  g_sphases=p_iphases;
  // reset state
  g_ccs_st=OFF0;
}



// ccs state machine
void ccs_cb(void) {
  static uint16_t toutA;
  static uint16_t toutB;
  static uint16_t toutC;
  static uint16_t toutW;
  static uint16_t toutP;
  static int16_t  amaxcur;
  static int16_t  aphases;
  static int16_t  tphases;
  static int16_t  cur1;
  static int16_t  cur2;
  static int16_t  cur3;

  // shape external control
  if(g_smaxcur>p_imaxcur) g_smaxcur=p_imaxcur;
  if(g_smaxcur>p_imaxcur) g_smaxcur=p_imaxcur;
  
  // state OFF0: power down, wait if so required 
  if(g_ccs_st==OFF0) {
    g_button=false;
    if(g_cpcurrent!=0) {
      g_ccs_st=OFF1;
      WRITE_CCSS("% OFF0 -> OFF1");
      toutP=g_systicks+3000;
    } else {
      g_ccs_st=OFF2;
      WRITE_CCSS("% OFF0 -> OFF2");
    }
  }
  // state OFF1: wait 10sec to power down
  if(g_ccs_st==OFF1) {
    cpcurrent(0);
    if(TRIGGER_SCHEDULE(toutP)) {
      WRITE_CCSS("% OFF1 -> OFF2");
      g_ccs_st=OFF2;
    }
  }    
  // state OFF2 (EV idle): open lock
  if(g_ccs_st==OFF2) {
    ssr(0);
    rms(0);
    cpcurrent(0);
    pilots(0);
    sigrel(0);
    lock(0);
    g_button=false;
    aphases=0;
    tphases=0;
    amaxcur=0;
    if(g_lock_st==open) {
      WRITE_CCSS("% ccs state: OFF2 -> OFF3");
      g_ccs_st=OFF3;
    }  
  }  
  // state OFF3: wait for operator --> state A
  if(g_ccs_st==OFF3) {
    if(g_button) {
      WRITE_CCSS("% OFF1 -> A0");
      g_ccs_st=A0;
    }
  }
  // state OFF9: wait for operator --> state OFF0
  if(g_ccs_st==OFF9) {
    ssr(0);
    rms(0);
    cpcurrent(0);
    pilots(0);
    sigrel(0);
    if(g_button) {
      WRITE_CCSS("% OFF9 -> OFF0");
      g_ccs_st=OFF0;
    }
  }  
  // state A0 (idle): enable CP at 100% for const +12V
  if(g_ccs_st==A0) {
    ssr(0);
    rms(0);
    pilots(1);
    cpcurrent(0);
    sigrel(1);
    g_button=false;
    aphases=0;
    amaxcur=0;
    toutA=g_systicks+30000;
    WRITE_CCSS("% A0 -> A1");
    g_ccs_st=A1;
  }  
  // state A1 (idle): validate EV present, i.e., CP to fall to 9V and appropriate PP -> state B (or timeout)
  if(g_ccs_st==A1) {
    if((g_cpilot==9) && (g_ppilot>0)) {
      WRITE_CCSS("% A1 -> B0");
      g_ccs_st=B0;
    }
    if(TRIGGER_SCHEDULE(toutA)) {
      WRITE_CCSS("% A1 -> OFF9 (timeout)");
      g_ccs_st=OFF9;
    }
    if(g_button) {
      WRITE_CCSS("% A1 -> OFF0 (button)");
      g_ccs_st=OFF0;
    }
  }  
  // state B0 (EV present): lock
  if(g_ccs_st==B0) {
    ssr(0);
    rms(0);
    pilots(1);
    sigrel(1);
    lock(1);
    g_button=false;
    aphases=0;
    amaxcur=0;
    if(g_lock_st==closed) {
      if((g_smaxcur>=60) && (g_ppilot>=60) && (g_sphases!=0)) {
	toutB=g_systicks+10000;
        WRITE_CCSS("% B0 -> B1");    
        g_ccs_st=B1;
      } else {
        WRITE_CCSS("% B0 -> W0 (invalid power)");    
        g_ccs_st=W0;
      }
    }
  }
  // state B1 (EV present and locked): wait for vehicle ready to charge, i.e., CP falls to 6V --> state C (or timeout --> state OFF)
  if(g_ccs_st==B1) {
    amaxcur=g_smaxcur;
    if(amaxcur>g_ppilot) amaxcur=g_ppilot;
    cpcurrent(amaxcur);
    if(g_cpilot==6) {
      WRITE_CCSS("% B1 -> C0/1");
      g_ccs_st=C0;
    }
    if(g_button) {
      WRITE_CCSS("% B1 -> OFF0 (button)");
      g_ccs_st=OFF0;
    }
    if(TRIGGER_SCHEDULE(toutB)) {
      WRITE_CCSS("% B1 -> W0 (unhappy with CP-PWM)");
      g_ccs_st=W0;
    }
  }
  // state C0 (EV about to charge): sanity checks
  if(g_ccs_st==C0) {
    g_button=false;
    if( (amaxcur<60) || (g_sphases==0) || (g_lock_st!=closed) ) {
      WRITE_CCSS("% C0 -> ERR");
      g_ccs_st=ERR0;
    } else {
      WRITE_CCSS("% C0 -> C1");
      g_ccs_st=C1;
    }
  }
  // state C1 (EV charging)
  if(g_ccs_st==C1) {
    aphases=g_sphases;
    cpcurrent(amaxcur);
    sigrel(1);
    ssr(aphases);
    rms(1);
    pilots(1);
    WRITE_CCSS("% C1 -> C2");
    g_ccs_st=C2;
    toutC=g_systicks+30000;
    tphases|=g_ssrphases_bin;;
    cur1=-1;
    cur2=-1;
    cur3=-1;
  }  
  // state C2 (EV charging, first 30secs): sense whether current/phases are accepted
  if(g_ccs_st==C2) {
    if(g_cur1>cur1) cur1=g_cur1;
    if(g_cur2>cur2) cur2=g_cur2;
    if(g_cur3>cur3) cur3=g_cur3;
    if((g_cpilot==9) && (cur1+cur2+cur3<45) && (g_ssrphases_bin!=0x07)) {
      WRITE_CCSS("% C2 -> P0 (phases rejected)");
      g_ccs_st=P0;
    }
    if(g_cpilot!=6) {
      WRITE_CCSS("% C2 -> OFF9 (cpilot)");
      g_ccs_st=OFF9;
    }
    if(g_button) {
      WRITE_CCSS("% C2 -> OFF0 (button)");
      g_ccs_st=OFF0;
    }    
    if(TRIGGER_SCHEDULE(toutC)) {
      if((cur1+cur2+cur3<45) && (g_ssrphases_bin!=0x07)) {
        WRITE_CCSS("% C2 -> P0 (phases rejected)");
        g_ccs_st=P0;
      } else {
        WRITE_CCSS("% C2 -> C3");
        g_ccs_st=C3;
      }	
    }  
  }
  // state C3 (EV charging): sense vehicle can't charge no more, i.e., CP raises to 9V --> state OFF (or button press)
  if(g_ccs_st==C3) {
    if(g_cpilot!=6) {
      WRITE_CCSS("% C3 -> OFF9 (cpilot)");
      g_ccs_st=OFF9;
    }
    if(g_button) {
      WRITE_CCSS("% C3 -> OFF0 (button)");
      g_ccs_st=OFF0;
    }
    // update configuration: low power
    if((g_sphases==0) || (g_smaxcur<60) || (g_ppilot<60)) {
      amaxcur=0;
      aphases=0;
      WRITE_CCSS("% C3 -> P0 (invalid power)");
      g_ccs_st=P0;
    } else { 
    if(g_sphases!=aphases) {
      amaxcur=0;
      WRITE_CCSS("% C3 -> P0 (phase change)");
      g_ccs_st=P0;
    } else { 
      amaxcur=g_smaxcur;
      if(amaxcur>g_ppilot) amaxcur=g_ppilot;
      cpcurrent(amaxcur);
      ssr(aphases);
    }}
  }
  // state P0 (EV prepare to pause charging): set timer to pause charging in 10sec
  if(g_ccs_st==P0) {
    cpcurrent(0);
    g_button=false;
    toutP=g_systicks+10000;
    WRITE_CCSS("% P0 -> P1");
    g_ccs_st=P1;
  }
  // state P1 (EV prepare to pause charging): pause charging
  if(g_ccs_st==P1) {
    if(g_button) {
      WRITE_CCSS("% W0 -> OFF");
      g_ccs_st=OFF0;
    }  
    if((TRIGGER_SCHEDULE(toutP)) || (g_cpilot!=6)) {
      toutW=g_systicks+10000;
      WRITE_CCSS("% P1 -> W0");
      g_ccs_st=W0;
    }
  }    
  // state W0 (EV idle): idle for 10secs
  if(g_ccs_st==W0) {
    ssr(0);  
    sigrel(0);
    rms(0);
    g_button=false;
    if(g_button) {
      WRITE_CCSS("% W0 -> OFF");
      g_ccs_st=OFF0;
    }  
    if(TRIGGER_SCHEDULE(toutW)) {
      WRITE_CCSS("% W0 -> W6/7/8/9");
      // came here for invalid power (from C3 or B0)
      if((amaxcur==0) && (aphases==0)) g_ccs_st=W6;
      // came here for unhappy with CP PWM (from B1)
      if((amaxcur!=0) && (aphases==0)) g_ccs_st=W7;
      // came here for phase change detect (from C3)
      if((amaxcur==0) && (aphases!=0)) g_ccs_st=W8;
      // came here for phases not accepted (C2)
      if((amaxcur!=0) && (aphases!=0)) g_ccs_st=W9;
    }
  }
  // state W6/7/8/9: abort via operator buttion
  if((g_ccs_st>=W6) && (g_ccs_st<=W9)) {
    if(g_button) {
      WRITE_CCSS("% W6/7/8/9 -> OFF");
      g_ccs_st=OFF0;
    }
  }  
  // state W6 (EV idle): wait for valid power allocation
  if(g_ccs_st==W6) {    
    if((g_smaxcur>=60) && (g_ppilot>=60) && (g_sphases!=0)) {
      WRITE_CCSS("% W6 -> A0");
      g_ccs_st=A0;
    }
  }
  // state W7 (EV idle): wait for higher CP PWM 
  if(g_ccs_st==W7) {
    if((amaxcur<100) && (g_smaxcur>=100) && (g_ppilot>=100) && (g_sphases!=0)) {
      WRITE_CCSS("% W7 -> A0");
      g_ccs_st=A0;
    }
  }
  // state W8 (EV idle): implement phase change (same as W6)
  if(g_ccs_st==W8) {    
    if((g_smaxcur>=60) && (g_ppilot>=60) && (g_sphases!=0)) {
      WRITE_CCSS("% W9 -> A0");
      g_ccs_st=A0;
    }
  }
  // state W9 (EV idle): wait for phase reconfiguratiom unless all phases are exhausted
  if(g_ccs_st==W9) {
    if((g_smaxcur>=60) && (g_ppilot>=60) && (g_sphases!=0) && (g_sphases!=aphases) && (tphases==0x07)) {
      WRITE_CCSS("% W9 -> A0");
      g_ccs_st=A0;
    }
  }
  // error state (protocol time outs)
  if(g_ccs_st==ERR0) {
    g_error|=ERR_CCS;
  }  
  // update cli variant of state
  g_ccss_cli=g_ccs_st;
}  


// report relevant status update
void report_cb() {
  static int16_t rccss=-1;
  static int16_t rcur1=-1;
  static int16_t rcur2=-1;
  static int16_t rcur3=-1;
  const char* parname=NULL;
  int16_t parval;
  // bail out if line is busy
  if(!serial_write_ready()) return;
  // figure change of CCS state
  if((parname==NULL) && (g_ccs_st!=rccss)) {
    parname="ccss";
    parval=g_ccs_st;
    rccss=g_ccs_st;
  }
  // figure change of current reading (allow for 0.5A tolerance)
  if((parname==NULL) && (g_cur1!=rcur1)) {
    if((g_cur1>rcur1+5) || (g_cur1<rcur1-5) || (g_cur1<0) || (rcur1<0)) {  
      parname="cur1";
      rcur1=g_cur1;
      parval=g_cur1;
    }  
  } 
  if((parname==NULL) && (g_cur2!=rcur2)) {
    if((g_cur2>rcur2+5) || (g_cur2<rcur2-5) || (g_cur2<0) || (rcur2<0)) {  
      parname="cur2";
      rcur2=g_cur2;
      parval=g_cur2;
    }
  } 
  if((parname==NULL) && (g_cur3!=rcur3)) {
    if((g_cur3>rcur3+5) || (g_cur3<rcur3-5) || (g_cur3<0) || (rcur3<0)) {  
      parname="cur3";
      rcur3=g_cur3;
      parval=g_cur3;
    }
  } 
  // write status update to the serial line
  if(parname!=NULL) {      
    serial_write('['); 
    serial_write_str(parname);
    serial_write('=');
    serial_write_int(parval);
    serial_write(']');
    serial_write_eol();
  }  
}

/*
*************************************************************************
*************************************************************************
*************************************************************************

command line parser
- format is "<PAR>?\r\n" for get and "<PAR>=<VAL>\r\n" for set
- both, get and set reply with <PAR>=<VAL>\r\n", or "parse-error"
- syntactic sugar "<PAR>!" to set the parameter to 1 (e.g. trigger some action)
- syntactic sugar "<PAR>~" to set the parameter to 0 
- the parser converts the string argument to functions/addresses to call/read/write
- dedicated special commands are implemented on top (eg plain '?' for "list all")

*************************************************************************
*************************************************************************
*************************************************************************
*/

// help text
const char h_version[]  PROGMEM = "read out firmware version";
const char h_blinks[]   PROGMEM = "overwrite status indicator (remote host)";
const char h_xblinks[]  PROGMEM = "overwrite status indicator (ESP32)";
const char h_time[]     PROGMEM = "read/set systime in [ms]";
const char h_temp[]     PROGMEM = "read AVR die temperatur [C]";
const char h_vcc[]      PROGMEM = "read AVR VCC [mV]";
const char h_ccss[]     PROGMEM = "read CCS state, see typedef \"ccs_state_t\"";
const char h_cmaxcur[]  PROGMEM = "read PP, i.e., max. cable capacity [100mA]";
const char h_amaxcur[]  PROGMEM = "read max. current signaled to EV via CP [100mA]";
const char h_aphases[]  PROGMEM = "read enabled phased as set by SSRs [decimal encoding])";
const char h_cur1[]     PROGMEM = "read current drawn on L1 [10mA]";
const char h_cur2[]     PROGMEM = "--- dito for L2";
const char h_cur3[]     PROGMEM = "--- dito for L3";
const char h_smaxcur[]  PROGMEM = "read/write dyn. max. current available [100mA]";
const char h_sphases[]  PROGMEM = "read/write dyn. phases allocated [decimal encoding]"; 
const char h_error[]    PROGMEM = "read error flags, see declaration \"g_error\"";
const char h_save[]     PROGMEM = "use \"save!\" to save configuration to EEPROM";
const char h_imaxcur[]  PROGMEM = "read/write installed max. current[100mA]";
const char h_iphases[]  PROGMEM = "read/write installed phases [decimal encoding]"; 
const char h_ccsdmp[]   PROGMEM = "use \"ccsdmp!\" to track CCS state progress";
const char h_sigrel[]   PROGMEM = "use \"sigrel!\"/\"sigrel~\" to en/disable the signal relay";
const char h_pilots[]   PROGMEM = "use \"pilots!\"/\"piloys~\" to en/disable periodic pilot reading";
const char h_cpilot[]   PROGMEM = "read CP [V]";
const char h_cpnslp[]   PROGMEM = "read CP on  nagativ halfwave [-V]";
const char h_ppilot[]   PROGMEM = "read PP (same as cmaxcur)[100mA]";
const char h_cpcur[]    PROGMEM = "read/write max. current indicated to the EV via PWM on CP";
const char h_rms[]      PROGMEM = "use \"rms!\"/\"rms~\" to en/disable periodic current reading";
const char h_rms1[]     PROGMEM = "get rms reading of CT1 [mV], use \"rms1!\" to trigger update"; 
const char h_rms2[]     PROGMEM = "--- dito for CT2";
const char h_rms3[]     PROGMEM = "--- dito for CT3";
const char h_rmsdmp[]   PROGMEM = "dump most recent rms record";
const char h_caldmp[]   PROGMEM = "use \"caldmp!\" for periodic dumps of RMS counts (req. \"rms!\")";
const char h_cala[]     PROGMEM = "read/write calibration parameter cA";
const char h_calb[]     PROGMEM = "read/write calibration parameter cB";
const char h_lock[]     PROGMEM = "use \"lock!\"/\"lock~\" to operate the lock";
const char h_lopnms[]   PROGMEM = "read/write pulse length to open lock [ms]";
const char h_lclsms[]   PROGMEM = "read/write pulse length to close lock [ms]";
const char h_ssr[]      PROGMEM = "operate contactors (dec. encoded), use \"ssr~\" for \"all off\"";
const char h_usage[]    PROGMEM = "use \"?\" or \"usage!\" to obtain this list";
const char h_reset[]    PROGMEM = "use \"reset!\" for a soft reset";

// set/get function types
typedef int16_t (*parsetfnct_t)(int16_t);
typedef int16_t (*pargetfnct_t)(void);


// name to token relation
typedef struct {
    const char* parstr;        // parameter name    
    const int16_t* pargetaddr; // address in memory for reading 
    int16_t* parsetaddr;       // address in memory for writing
    parsetfnct_t parsetfnct;   // function for writing
    const char* parhlp;        // help text
} partable_t;

// forward
int16_t usage(int16_t);


// name to token table (NULL-terminated --- should be in PROGMEM)
const partable_t partable[]={
  // normal operation
  {"ver",     &g_version,   NULL,        NULL,        h_version}, // g_version can be read from memory
  {"blinks",  &g_blinks,    &g_blinks,   NULL,        h_blinks},  // g_blinks can be read/written from/to memory
  {"xblinks", &g_xblinks,   &g_xblinks,  NULL,        h_xblinks}, // g_xblinks can be read/written from/to memory
  {"time",    &g_systime,   NULL,        &systime_set,h_time},    // g_systime has explicit setter
  {"temp",    &g_temp,      NULL,        NULL,        h_temp},    // g_temp can be read from memory
  {"vcc",     &g_vcc,       NULL,        NULL,        h_vcc},     // g_vcc can be read from memory
  {"ccss",    &g_ccss_cli,  NULL,        NULL,        h_ccss},    // g_ccs_st can be read from memory
  {"cmaxcur", &g_ppilot,    NULL,        NULL,        h_cmaxcur}, // get cable max current (same as PPilot)
  {"amaxcur", &g_cpcurrent, NULL,        NULL,        h_amaxcur}, // get actual max current as set on PWM
  {"aphases", &g_ssrphases, NULL,        NULL,        h_aphases}, // get actually enabled phase as set vua SSRs
  {"cur1",    &g_cur1,      NULL,        NULL,        h_cur1},    // read current phase L1
  {"cur2",    &g_cur2,      NULL,        NULL,        h_cur2},    // read current phase L2
  {"cur3",    &g_cur3,      NULL,        NULL,        h_cur3},    // read current phase L3
  {"sphases", &g_sphases,   &g_sphases,  NULL,        h_sphases}, // set dyn. enabled mains supply phases (decimal encoded)
  {"smaxcur", &g_smaxcur,   &g_smaxcur,  NULL,        h_smaxcur}, // set dyn. max supply mains current 
  {"error",   &g_error,     NULL,        NULL,        h_error},   // g_error can be read from memory
  // configure
#ifdef MODE_CONFIGURE  
  {"save",    NULL,         NULL,        &conf_save,  h_save},    // save parameters to eeprom
  {"iphases", &p_iphases,   &p_iphases,  NULL,        h_iphases}, // set installed phases (decimal encoded)
  {"imaxcur", &p_imaxcur,   &p_imaxcur,  NULL,        h_imaxcur}, // set installed max supply mains current 
#endif  
  // first installation
#ifdef MODE_INSTALL  
  {"ccsdmp",  NULL,         &p_ccsdmp,   NULL,        h_ccsdmp},  // "ccsdmp!" enables tracking of the CCS state
  {"sigrel",  NULL,         NULL,        &sigrel,     h_sigrel},  // operate signal rellay
  {"pilots",  &g_pilots,    NULL,        &pilots,     h_pilots},  // en/disable periodic pilot reading
  {"cpilot",  &g_cpilot,    NULL,        NULL,        h_cpilot},  // ctrl pilot read-back [V]
  {"cpnslp",  &g_cpnslp,    NULL,        NULL,        h_cpnslp},  // ctrl pilot read-back on low slope, aka "diodtest"
  {"ppilot",  &g_ppilot,    NULL,        NULL,        h_ppilot},  // prox pilot read-back [100mA]
  {"cpcur",   &g_cpcurrent, NULL,        &cpcurrent,  h_cpcur},   // set PWM via setter, get from memory [100mA]
  {"rms",     &g_rms,       NULL,        &rms,        h_rms},     // en/disable periodic RMS updates
  {"rms1",    &g_rms1,      NULL,        &rms1_start, h_rms1},    // "rms1!" to trigger a single measurement on phase L1
  {"rms2",    &g_rms2,      NULL,        &rms2_start, h_rms2},    // -- dito L3
  {"rms3",    &g_rms3,      NULL,        &rms3_start, h_rms3},    // -- dito L3
  {"rmsdmp",  NULL,         NULL,        &rms_dump,   h_rmsdmp},  // "rmsdmp!" dumps recent RMS record 
  {"caldmp",  NULL,         &p_caldmp,   NULL,        h_caldmp},  // "caldmp!" enables periodic output for rms calibration
  {"cala",    &p_cala,      &p_cala,     NULL,        h_cala},    // calibration parameter cA
  {"calb",    &p_calb,      &p_calb,     NULL,        h_calb},    // calibration parameter cB  
  {"lock",    NULL,         NULL,        &lock,       h_lock},    // operate lock
  {"lopnms",  &p_lopnms,    &p_lopnms,   NULL,        h_lopnms},  // time to open lock
  {"lclsms",  &p_lclsms,    &p_lclsms,   NULL,        h_lclsms},  // time to close lock
  {"ssr",     NULL,         NULL,        &ssr,        h_ssr},     // "ssr=123" to activate all SSRs
  {"usage",   NULL,         NULL,        &usage,      h_usage},   // list all parameters
  {"reset",   NULL,         NULL,        &reset,      h_reset},   // softreset "reset!"
#endif  
  // end of table
  {NULL, NULL, NULL, NULL,        NULL},            
};


// list all parameters to serial line (tuned to set sync writeln)
void serial_write_parlist(void) {
  // iterate table
  const partable_t* ptab=partable;
  while(ptab->parstr!=NULL) {
    serial_write_pstr("% ");
    serial_write_str(ptab->parstr);
    if(ptab->pargetaddr) {
      serial_write('=');
      serial_write_int(*ptab->pargetaddr);
    }
    serial_write_tab(12);
    if(ptab->pargetaddr && (ptab->parsetaddr || ptab->parsetfnct)) {
      serial_write_pstr("[R/W]");
    }  
    if(!ptab->pargetaddr && (ptab->parsetaddr || ptab->parsetfnct)) {
      serial_write_pstr("[-/W]");
    }  
    if(ptab->pargetaddr && !(ptab->parsetaddr || ptab->parsetfnct)) {
      serial_write_pstr("[R/-]");
    }  
    if(ptab->parhlp) {
      serial_write_pstr(" % ");
      serial_write_pgmstr(ptab->parhlp);
    }
    serial_write_eol();
    ++ptab;
  }
}  


// parse command line, take appropriate action, return true on success
bool parse(char* line){
  DBW_CLI(serial_write_pln("% parse A"));
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
  DBW_CLI(serial_write_pln("% parse B"));
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
  DBW_CLI(serial_write_pln("% parse C"));
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
    DBW_CLI(serial_write_pstr("% parse N:"));
    DBW_CLI(serial_write_ln(pos));
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
  DBW_CLI(serial_write_pln("% parse D"));
  // if its a get ..
  if(sep=='?') {
    if(ptab->pargetaddr) val=*ptab->pargetaddr;
    else return false;
  }
  DBW_CLI(serial_write_pln("% parse E"));
  // if its a set ..
  if(sep=='=') {
    if(ptab->parsetfnct) val=(*ptab->parsetfnct)(val);
    else
      if(ptab->parsetaddr) *ptab->parsetaddr=val;
      else
	return false;
  }
  DBW_CLI(serial_write_pln("% parse F"));
  // reply
  serial_write_str(line);
  serial_write('=');
  serial_write_int(val);
  serial_write_eol();
  return true;
}

// convenience command
int16_t usage(int16_t doit) {
    if(!doit) return 0;
    serial_write_pln("usage=1");
    serial_writeln_sync();
    serial_write_pln("% [[[");
    serial_write_pln("% =======");
    serial_write_pstr("% AGCCS-Ctrl22C AVR Firmware Version ");
    serial_write_uint(g_version/10);
    serial_write('.');
    serial_write_uint(g_version%10);
    serial_write_eol();
    serial_write_pln("% Copyright Thomas Moor (c) 2020, 2021"); 
    serial_write_pln("% =======");
    serial_write_parlist();
    serial_write_pln("% =======");
    serial_write_monitor();
    serial_write_pstr("% systime: ");
    serial_write_uint(g_systime);
    serial_write_eol();
    serial_write_pstr("% lock state: ");
    serial_write_uint(g_lock_st);
    serial_write_eol();
    serial_write_pstr("% ccs state: ");
    serial_write_uint(g_ccs_st);
    serial_write_eol();
    serial_write_pln("% =======");
    serial_write_pln("% ]]]");
    serial_writeln_async();
    g_cycleskip=true;
    return 1;
}

// overall command line interface (aka std parameter set/get + some special commands)
void cmdline(char* ln) {
  // special command: list all status
  if(*ln=='?') {
    usage(1);
    return;
  }
  // std get/set parameter 
#ifdef DEGUGGING
  serial_writeln_sync();   // make sure that we can reply
#endif
  bool ok=parse(ln);
  if(!ok) serial_write_pln("parse error");
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


// its a boy
int main(){

  // initialize our hardware
  clock_init();
  serial_init();
  ledbutton_init();
  dio_init();
  vref_init();
  adc_init();
  rms_init();
  cppwm_init();
  systime_init();

  // start interrupts now
  CPU_SREG |= CPU_I_bm;

  // read configuratiom
  conf_load();

  // say hello
  serial_write_pln("% AGCCS-Ctrl22C --- enter \"?[CR/LF]\" for more details");
  
  // run forever (target for less than 10ms cycle time)
  while(1){

    // aux callbacks for status updates
    systime_cb();     // update systime and systicks
    monitor_cb();     // monitor cycle time
    led_blinks_cb();  // flash led
    lock_cb();        // operate lock via external state g_lock_state
    bot_cb();         // check for brown out
    button_cb();      // sense positive edge on button via g_button
    rms_cb();         // keep updating asynchronous current meassurement (max 5ms)
    pilots_cb();      // keep updating synchronous analog reading of pilots (max 1.5ms)
    env_cb();         // keep updating synchronous analog reading of temperature and VCC
    report_cb();      // report status updates to serial line

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
      // lock error: if the lock is open we recover from in 10sec
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

    // cancel operator button
    g_button=false;    
    
  } // loop forever

  // never get there
  return(0);
}




