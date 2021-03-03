/* 
===========================================================================
===========================================================================
The below code has been developed from scratch to meet its specifc purpose. 
However, the buiding blocks of basic functionality have been derived 
from the examples provided by the ESP-IDF and ESP-MDF, in particuar
- the Mupgrade example
- the TCP-client example
- the UART echo example
We hence inherit the Apache License, Version 2.0, and the code can be 
used/modified/distributed under that license:

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

(c) Thomas Moor, 2020-2021
===========================================================================
===========================================================================
*/

/*  
===========================================================================
===========================================================================
We address the following overall setup

[Host]<=>[WLAN AP]<=>(ESP Mesh Network)<=>[ESP32 nodes]<=>(RS232)<=>[AVR target uC]
                            ^^===========>[ESP32 nodes]<=>(RS232)<=>[AVR target uC]
                            ^^                ...                       ...
                            ^^===========>[ESP32 nodes]<=>(RS232)<=>[AVR target uC]

In our specfic setup, the [Host] and the [WLAN AP} are realised as one
unit by a Raspberry Pi. Likewise, each [ESP32] node together with one
AVR uC form a single hardware component, which in our use case is integrated
with an EV charging station. In contrast to the one and only [Host], we use
the (ESP Mesh Network) to hook up a "large number" of [ESP32 nodes] with
one [AVR uC] each. We comment on the roles of the individual components.

[AVR target uC]
- The uC physically interacts with its environment in a real-time fashion to 
  implement safety critical functionality. This is the final payload of our
  entire setup.
- The uC behaviour is parametrised by a set of 32bit integer variables,
  accessible with a human readable get/set line based protocoll over a RS232 
  serial interface. This protocol shall be tunneled for the [Host] to 
  coordinate/control all [AVR target uCs] in the network. 
- The same RS232 interface can be used to update the AVR firmware
  via a bootloader with the Ardiono Optiboot protocol. This functionality
  shall also be accessible to the [Host].

[ESP32 node]
- All [ESP32 nodes] communicate via the ESP Mesh Network implemented by the
  ESP-MDF SDK. I.e., there is one dynamcally elected root node which effectively can
  interchange messages with every other node 
- The unique root node can access the [Host] via IP WLAN. On this end, the root 
  behaves as a TCP client and expects a TCP server on the [Host] to accept one 
  incomming connection on a specific port. The root node accepts JSON encoded commands 
  in the format {"dst":"xyz", "cmd":...}, dispatches them to the respective
  destination node "xyz" and awaits the reply to be forwarded to the [Host]. 
- Using the boradcast address "*" and the command "status", each node replys by 
  a JSON encoded status, including its address. Hence, the [Host] can figure the 
  network topology.
- When a node receives a message with command "avrgetpar", it queries the respective
  parameter from the [AVR target uC] via RS232 and returns the value to the
  root for fowarding to the [Host]. Likewise, there is a "avrsetpar" command.  
- There is also a command "avrimage" for the host to downstream firmware for
  the [AVR target uC]. This is organised in small packeges, which are 
  buffered in a specific partition of the [ESP32]. Once the firmware has
  been received completely, the [ESP32] can re-programm the [AVR target uC] via
  RS232 using the Optiboot protocol 
- The [ESP32 nodes] implement the OTA firmware upgrade as proposed by the MDF.
  Hence, the [Host] can downstream firmware upgrades for the ESP32s via this mechanism.
  Technically, it sends the specfig command "upgrade" to the root node which in turn
  downloads the firmware from the [Host] via http. The firmware is then distributed
  over all nodes in the mesh network (largely by "ESP-MDF magic"). 

[Host/WLAN SP]
- The implemented protocolls allow to manage the distribution of firmware both for 
  the [ESP32] nodes and the attached [AVR target uCs]. We refer to this as a networked 
  firmware management system.
- In normal operation, the [Host] can also control the operation of the individial
  [AVR target uCs] by setting/getting parameter values via the [ESP32 nodes].

===========================================================================
===========================================================================
*/

// firmware version string (relevant format for OTA: "OneDigit.OneDigit", nothing else tested)
#define DEMESH_VERSION "6.3"

 
// minimum includes
#include "mdf_common.h"
#include "mwifi.h"
#include "mupgrade.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "mbedtls/base64.h"
#include "esp32/rom/crc.h"

// maths only needed for load emulator   
#include "math.h"  

// includes for softap
#include "esp_wifi.h"

// mqtt protocol  
#include "mqtt_client.h"



// DEMESH globals
static esp_netif_t *g_netif_sta = NULL;
static int g_sockfd    = -1;
static struct sockaddr_in g_server_addr;
static const char *TAG = "demesh";
static int g_blinks = 0;
static int g_upgrade_stage  = 0;
static char* g_upgrade_version=NULL; 
static char* g_upgrade_board=NULL;
static esp_mqtt_client_handle_t g_mqtt_client=NULL;
static int g_mqtt_stop=0;


// DEMESH globals, local copy of target uC state for heartbeat
// Note: if it was not for target simulation, we should be transparent regarding the target state
static int g_heartbeat_period=5000;  // period to send heartbeat in ms
static int g_tstate_version=0;    // target uC firmware version 
static int g_tstate_ccss=0;       // state in CCS charging scheme (0<>idle ... 30<> charging, >=50 error)
static int g_tstate_phases=0;     // phases to operate  
static int g_tstate_maxcur=0;     // max alllowed current per phase (unit 100mA, aka "10" reads "1A")
static int g_tstate_cur1=0;       // actual current drawn on phase 1 (unit 100mA %)
static int g_tstate_cur2=0;       // actual current drawn on phase 1 (unit 100mA %)
static int g_tstate_cur3=0;       // actual current drawn on phase 1 (unit 100mA %)


// DEMESH globals, local copy of target uC control parameters (fake when simulating)
// Note: if it was not for target simulation, we should be transparent regarding the target control
static int g_tcontrol_blinks=0;       // operator button flash light (not simulated)
static int g_tcontrol_maxcur=0;       // mac allowed current (unit 100mA)
static int g_tcontrol_phases=0;       // enabled phases (i.e. "1" for use phase 1 only) 


// DEMESH globals, debug server
static bool g_debug_target=false;
static int g_sockfd_telnet    = -1;
static int g_sockfd_optiboot    = -1;
static int g_station_cnt    = 0;

// convenience macros to printf the least significant bytes of a MAC address (Mesh Id)
#define LSDMAC2STR(a) (a)[2], (a)[3], (a)[4], (a)[5]
#define LSDMACSTR "%02x:%02x:%02x:%02x"

// convenience macros to printf the MAC address without colons (MQTT topic)
#define NOCMAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define NOCMACSTR "%02x%02x%02x%02x%02x%02x"

// forward declr.: query our very own system time (ms, defined rollover, synched ... code given below)
static TickType_t systime(void);

// forward declr.: trigger heartbeat (publish MQTT and send over plain TCP socket) 
static void heartbeat_trigger(void);


/*
===========================================================================
===========================================================================
Hardware configuration: we provide a limited number of hardware configurations,
scaling  from "nothing", "blink a LED" to "M5Stick with bells and whistles".
Most likely, you will need to adapt this to your needs. Use "nope" as a
template, inspect "m5stick" for inspiration to create your own.
===========================================================================
===========================================================================
*/


/*
 **********************************************************************
 **********************************************************************
 Hardware configuration: any ESP32, we dont touch your peripherials

 "No peripherials" translates "no feedback". You dont realy  want this.
 Use it as a templete to handle your hardware, at least a LED to flash
 is a realistic minimum during deveopment. If you have/need more, see
 below the code specific to the M5Stick.

 [The hardware configuration is conveniently chosen via "make menuconfig"]
 **********************************************************************
 **********************************************************************
 */


#ifdef CONFIG_TARGET_BOARD_NOPE

#define DEMESH_BOARD "nope"            

#undef AVR_PRESENT                           // dont have anything, even no AVR attached


// you have at least an LED to flash? enter here
//#define BLINK_GPIO GPIO_NUM_2             // GPIO pad to flash ...
//#define BLINK_ON 1                        // ... and its active on

void init_devices(void)
{
    // a) do a hardware init here, e.g., set-up output GPIOs, configure serial coms
    // b) set up RTOS to operate your hardware (everything except the provided
    //    "blink" needs dedicated code --- see M5Stick for an example.
}

#endif

  

/*
 **********************************************************************
 **********************************************************************
 Hardware configuration: M5StickC  (LED at GPIO10, LCD, ButtonA/B)

 The M5StickC is a neat litle ESP32 bases device. We use the built in
 OLED display to monitor global variables, e.g., display our mac address
 and given some minimum info on the mesh topology. We use the
 built-in LED to flash debugging and/or status information, this is provided 
 as part if demesh's infrastructure. The OLED stuff etc is put in seperate
 RTOS tasks, cicked-off by "init_devices()". 

 [The hardware configuration is conveniently chosen via "make menuconfig"]
 **********************************************************************
 **********************************************************************
 */

#ifdef CONFIG_TARGET_BOARD_M5
#define DEMESH_BOARD "m5stick"

// includes to make the M5Stick work
#include "m5stickc.h"
#include "wire.h"
#include "AXP192.h"

// we do blink on GPIO10
#define BLINK_GPIO GPIO_NUM_10   
#define BLINK_ON 0

// we dont care about an attached AVR (because there is no such ...)
#undef  AVR_PRESENT

// set OLED brightness (stolen from the Arduino SDK)
static void AXP192ScreenBreath(uint8_t brightness)
{
    if (brightness > 12) brightness = 12;
    uint8_t buf = AxpRead8bit(&wire0,0x28);
    AxpWriteByte(&wire0,  0x28, ((buf & 0x0f) | (brightness << 4)) );
}

// uopdate economics
int g_screen_nua=true;
int g_screen_nub=true;

// update OLED with current mesh status (edit here to change content/layout)
static void status_screen_update_a(void)
{
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    float bat_voltage;
    float bat_current;

    TFT_setFont(SMALL_FONT, NULL);
    _fg=TFT_GREEN;
    _bg=TFT_BLACK;
 
    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_mesh_get_parent_bssid(&parent_bssid);
    bat_voltage = AXP192GetBatVoltage(&wire0);
    bat_current = AXP192GetBatCurrent(&wire0);
     
    char strbuff[50];
    if(g_upgrade_stage==0) {
        sprintf(strbuff, "DEMESH  v%s                 ",DEMESH_VERSION);
    } else {
        sprintf(strbuff, "DEMESH  (upgr. #%d)         ",g_upgrade_stage);
    }
    TFT_print(strbuff, 5, 0);
    sprintf(strbuff, "Bat.:   %.2f/%.0f    ", bat_voltage,-bat_current);
    TFT_print((char *)strbuff, 5, 12);
    sprintf(strbuff, "SysId:  " LSDMACSTR "     ", LSDMAC2STR(sta_mac));
    TFT_print(strbuff, 5, 24);
    if(mwifi_is_connected()) {
        sprintf(strbuff, "Parent: " LSDMACSTR "   ", LSDMAC2STR(parent_bssid.addr));
    } else {
        sprintf(strbuff, "Parent: no mesh    ");
    }  
    TFT_print(strbuff, 5, 36);
    if(mwifi_is_connected()) {
        sprintf(strbuff, "Nodes:  #%d (L%d)   ", esp_mesh_get_total_node_num(),esp_mesh_get_layer());
    } else {
        sprintf(strbuff, "Nodes:  no mesh    ");
    }      
    TFT_print(strbuff, 5, 48);
    sprintf(strbuff, "Time:   %d     ", systime());
    TFT_print(strbuff, 5, 60);
    g_screen_nub=true;
}


// update OLED with current target uC status (edit here to change content/layout)
// (old version with large digits ... keeping this for reference)
/*
static void status_screen_update_b(void)
{
    static int rmaxcur=0;
    static int rcur=0;
    g_screen_nub = g_screen_nub || (g_tstate_maxcur!=rmaxcur);
    g_screen_nub = g_screen_nub || (g_tstate_cur!=rcur);
    if(!g_screen_nub) return;
    rmaxcur=g_tstate_maxcur;
    rcur=g_tstate_cur;
    g_screen_nub=false;
    char strbuff[5];
    TFT_setFont(FONT_7SEG, NULL);
    set_7seg_font_atrib(16, 4, 1, TFT_YELLOW);
    _fg=TFT_YELLOW;
    _bg=TFT_BLACK;
    sprintf(strbuff, "%02d",rcur);
    TFT_print(strbuff, 10, 10);
    set_7seg_font_atrib(16, 4, 1, TFT_RED);
    _fg=TFT_RED;
    _bg=TFT_BLACK;
    sprintf(strbuff, "%02d",rmaxcur);
    TFT_print(strbuff, 82, 10);
}
*/

// update OLED with current target uC status (edit here to change content/layout)
static void status_screen_update_b(void)
{
    TFT_setFont(SMALL_FONT, NULL);
    _fg=TFT_RED;
    _bg=TFT_BLACK;    
    char strbuff[50];
    sprintf(strbuff, "CTRL22 v%.1f             ",g_tstate_version/10.0);
    TFT_print(strbuff, 5, 0);
    sprintf(strbuff, "CCSState: %2d[P=%03d]", g_tstate_ccss, g_tstate_phases);
    TFT_print((char *)strbuff, 5, 12);
    sprintf(strbuff, "MaxCur:        %4.1f", g_tstate_maxcur/10.0);
    TFT_print((char *)strbuff, 5, 24);
    sprintf(strbuff, "Current 1:     %4.1f", g_tstate_cur1/10.0);
    TFT_print(strbuff, 5, 36);
    sprintf(strbuff, "Current 2:     %4.1f", g_tstate_cur2/10.0);
    TFT_print(strbuff, 5, 48);
    sprintf(strbuff, "Current 3:     %4.1f", g_tstate_cur3/10.0);
    TFT_print(strbuff, 5, 60);   
    g_screen_nua=true;
}    


// Toggle between screen A and screen B via button A. When screen A is shown time out after 5secs.
// See also be,ow "button_event".
static char g_screen='0';
static void status_screen_task(void* arg) {
    int i;
    char rscreen=g_screen;
    g_screen='a';
    TFT_fillScreen(TFT_BLACK);
    AXP192ScreenBreath(12);
    for( i=50; i>0; --i ) {
        if( g_screen != rscreen) {
	    rscreen=g_screen;
	    TFT_fillScreen(TFT_BLACK);
	}   
        if( rscreen=='a' ) status_screen_update_a();
        if( rscreen=='b' ) status_screen_update_b();
        vTaskDelay(200 / portTICK_RATE_MS);
        if( (gpio_get_level(BUTTON_BUTTON_A_GPIO)==0) || (rscreen=='b') ) i=50;
    }  
    AXP192ScreenBreath(0);
    g_screen='0';
    vTaskDelete(NULL);
}


// Sense buttons A and B
static void buttonEvent(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    // button A controls the screen
    if((base == button_a.esp_event_base) && (id == BUTTON_PRESSED_EVENT)) {
        if( g_screen=='0' ) {
            xTaskCreate(status_screen_task, "status_screen_task", 4 * 1024,
	  	    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        } else {
        if( g_screen=='a' ) {
	    g_screen='b';
        } else {
	    g_screen='a';
        }}
    }
    // button B starts/stops charging simulation
    if((base == button_b.esp_event_base) && (id == BUTTON_PRESSED_EVENT)) {
      if(g_tstate_ccss==0) g_tstate_ccss=10;
	else g_tstate_ccss=0;
      heartbeat_trigger();
    }
}

// initialise M5Stick hardware (incl. register button A/B to kick-off OLED update)
static void init_devices(void)
{
    M5Init();

    esp_event_handler_register_with(event_loop, BUTTON_A_EVENT_BASE, BUTTON_PRESSED_EVENT, buttonEvent, NULL);
    esp_event_handler_register_with(event_loop, BUTTON_B_EVENT_BASE, BUTTON_PRESSED_EVENT, buttonEvent, NULL);
    esp_event_handler_register_with(event_loop, BUTTON_B_EVENT_BASE, BUTTON_RELEASED_EVENT, buttonEvent, NULL);

    char strbuff[50];
    font_rotate = 0;
    text_wrap = 0;
    font_transparent = 0;
    font_forceFixed = 0;
    gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(LANDSCAPE);
    TFT_resetclipwin();
    AXP192ScreenBreath(8);

    TFT_setFont(SMALL_FONT, NULL);
    sprintf(strbuff, "DEMESH v%s",DEMESH_VERSION);
    TFT_print(strbuff, 5, 0);
}


#endif



/*
 **********************************************************************
 **********************************************************************
 Hardware configuration: DOIT ESP32 dev-board (LED at GPIO2)

 This plain dev board comes with a LED which we use to blink debug
 codes and/or status information --- the bare minimum we need to
 keep track. We have an AVR target MCU attached though, as this is
 the purpose of this exercise.

 [The hardware configuration is conveniently chosen via "make menuconfig"]
 **********************************************************************
 **********************************************************************
 */

#ifdef CONFIG_TARGET_BOARD_GPIO2

#define DEMESH_BOARD "gpio2"            
#define BLINK_GPIO GPIO_NUM_2             // blink on GPIO2
#define BLINK_ON 1                        // active high
#define DEBUG_GPIO GPIO_NUM_0             // debug target avr (reuse bootmode button on dev-board)  
#define DEBUG_ON 0                        // active low 

#define AVR_PRESENT                       // we do have an AVR taget to care (undefine this if no AVR present)
#define AVR_TXD_GPIO  GPIO_NUM_17         // target AVR serial: pins to transmit
#define AVR_RXD_GPIO  GPIO_NUM_16         // target AVR serial: pins to receive
#define AVR_BAUDRATE  57600               // target AVR serial: baudrate (format always is "8N1", old arduino nano takes 57600)
#define AVR_RST_GPIO  GPIO_NUM_5          // target AVR: how to reset
#define AVR_RST_ACTIVE  0                 // target AVR: dont need to be active high for "run"
#define AVR_IMG_CNT     (31*1024)         // target AVR: size of firmware (e.g. arduino nano "32k - bootloader")
#define AVR_OPT_CNT     128               // target AVR: byte count per page write (128 bytes are common practice)


void init_devices(void)
{
    // avr reset pin (passive is fine)
    gpio_pad_select_gpio(AVR_RST_GPIO);
    gpio_set_direction(AVR_RST_GPIO, GPIO_MODE_INPUT);

    // debug select pin
    gpio_pad_select_gpio(DEBUG_GPIO);
    gpio_set_direction(DEBUG_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(DEBUG_GPIO, GPIO_PULLUP_ONLY);
    
    // wait to give user a chance to switch to debug
    vTaskDelay(1000 / portTICK_RATE_MS);

}

#endif



/*
 **********************************************************************
 **********************************************************************
 Hardware configuration: FGCCS board, rev 1.0

 This is our first revision of the EV CCS charging station  --- no fog, no
 humor, no blink (well, we do blink on GPIO_NUM_0 for those who care).

 [The hardware configuration is conveniently chosen via "make menuconfig"]
 **********************************************************************
 **********************************************************************
 */

#ifdef CONFIG_TARGET_BOARD_FGCCS_1_0

#define DEMESH_BOARD "fgccs10"

#define BLINK_GPIO  GPIO_NUM_0            // blink LED on programming adapter
#define BLINK_ON 0                        // blink LED is active low
#define DEBUG_GPIO  GPIO_NUM_0            // debug target avr  
#define DEBUG_ON 1                        // active low 0 (set to high 1 to boot into debug mode by default)

#define AVR_PRESENT                       // we do have an AVR taget to care
#define AVR_TXD_GPIO  GPIO_NUM_17         // target AVR serial: pins to transmit
#define AVR_RXD_GPIO  GPIO_NUM_16         // target AVR serial: pins to receive
#define AVR_BAUDRATE  115200              // target AVR serial: baudrate (always "8N1")
#define AVR_RST_GPIO  GPIO_NUM_12         // target AVR: how to reset
#define AVR_RST_ACTIVE  1                 // target AVR: be always active (incl. "high" for "run")
#define AVR_IMG_CNT     (47*1024)         // target AVR: size of firmware (e.g. ATMega4808  "48k - bootloader")
#define AVR_OPT_CNT     128               // target AVR: byte count per page write (128 bytes are common practice)
#define AVR_OPT_BADDR                     // target AVR: Optiboot_x uses byte addresses as opposed to word addresses
#define AVR_OPT_TADDR   0x200             // target AVR: offset for application code 0.5kByte (botloader)


void init_devices(void)
{
    // avr reset pin
    gpio_pad_select_gpio(AVR_RST_GPIO);
    gpio_set_direction(AVR_RST_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(AVR_RST_GPIO,1);

    // debug select pin
    gpio_pad_select_gpio(DEBUG_GPIO);
    gpio_set_direction(DEBUG_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(DEBUG_GPIO, GPIO_PULLUP_ONLY);
 
    // wait to give user a chance to switch to debug
    vTaskDelay(1000 / portTICK_RATE_MS);
}

#endif


/*
===========================================================================
===========================================================================
Hardwrare dealt with? Fine ... the actual code is structured as follows:

(a) system related low-level stuff; e.g. system time and debugging via "blink"
(b) payload aka talk-to/manage the attached target AVRs
(c) communicate with the host via TCP (using JSON encode messages)
(d) maintane our selfs, in particular the MDF Wifi Mesh and OTA firmware,

===========================================================================
===========================================================================
*/  


/*
 **********************************************************************
 **********************************************************************
 Syncronized system time
 - based on freeRTOS ticks obtained from xTaskGetTickCount,
   i.e. unsigned 32bit tick count in ms with rollover at 2^32ms
 - plus the positive offeset "g_systime_offset" to be set for 
   synchronisation (relies on 32bit 2s-complement maths);
 - plus the negative offset accumulated in "g_systime_offset" for 
   an explicit rollover after exactly 1h (so that we can have synced periodic
   tasks where the period is a divider of 3600000ms, e.g, 1000ms)
 **********************************************************************
 **********************************************************************
 */


// offset to sync time in unit "ticks"
static TickType_t g_systime_offset=0;

// rollover at 3600000ms aka 1h
#define DEMESH_ROLLOVER_TICKS (3600000 / portTICK_PERIOD_MS)

// query systime in unit ticks; need to do at least once an hour to keep track of rollover
static TickType_t systime(void) {
    TickType_t ticks;
    TickType_t ticks_mod1h;
    // nominal case
    ticks = xTaskGetTickCount() + g_systime_offset;
    // when more than one hour has past, adjust offset 
    if(ticks>=DEMESH_ROLLOVER_TICKS) {
        ticks_mod1h = ticks % DEMESH_ROLLOVER_TICKS;
        g_systime_offset -= ticks - ticks_mod1h;
        ticks=ticks_mod1h;
    }  
    return ticks;
}  

// last roundtrip when attempting to adjust
static TickType_t g_systime_roundtrip=0;

// set g_sytime_offset from parent time "t2" modulo network roundtrip
static int systime_adjust(TickType_t t1, TickType_t t2, TickType_t t3) {
    // At local systime t1 we ask our parent to read its systime to t2 and we
    // revceive their reply at local systime t3; assuming symmetric communication
    // delays, local systime was (t1+t3)/2 when the parent read their clock to t2
    // 1) sense/compensate role-over between t1 and t3
    if(t3<t1)
        t3+=DEMESH_ROLLOVER_TICKS;
    g_systime_roundtrip=t3-t1;
    // 2) reject if roundtrip took too long (more than 200ms -- needs tunig)
    if(g_systime_roundtrip > 200) {
        MDF_LOGI("systime_adjust: t1 %d, t2 %d, t3 %d -- rejected (%dms)", t1,t2,t3,g_systime_roundtrip);
        return 0;
    }  
    // 3) doit
    MDF_LOGI("systime_adjust: t1 %d, t2 %d, t3 %d -- accepted (%dms)", t1,t2,t3,g_systime_roundtrip);
    g_systime_offset += t2 - (t1+t3)/2;
    return 1;
}



/*
 **********************************************************************
 **********************************************************************
 Blink
 - status display on LED connected to GPIO, flashing 1 to 20 
   times in a period of 2secs
 - by default, we morse the connection state and the upgrade status
 - set "g_blinks" to a non-zero value to overwrite the default
   for debugging purposes
 **********************************************************************
 **********************************************************************
 */

#ifdef BLINK_GPIO 

void blink_task(void *arg) {
    int n=0;
    MDF_LOGI("Blink task is running");
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        if(!mwifi_is_connected()) n=3;
        if(mwifi_is_connected()) n=1;
        if(g_upgrade_stage>0) n=20;
        if(g_blinks>0) n=g_blinks;
        for(int i=n; i>0; --i ) {
            gpio_set_level(BLINK_GPIO, BLINK_ON);
            vTaskDelay(25 / portTICK_PERIOD_MS);
            gpio_set_level(BLINK_GPIO, 1-BLINK_ON);
            vTaskDelay(75 / portTICK_PERIOD_MS);
        }
	if(n<20) {
	  vTaskDelay( (2000 / portTICK_PERIOD_MS ) - ( systime() % (2000 / portTICK_PERIOD_MS) ) );
	}    
    }
}  

#else

void blink_task(void *arg) {
    while(1) {
        MDF_LOGI("Blink task is running -- however, no LED configured (%d)", g_blinks);
        vTaskDelay( 5000 / portTICK_PERIOD_MS );
    }
}

#endif

/*
===========================================================================
===========================================================================

Payload (caring attachd AVR) comes in two parts
I: set/get paremeters via a simple human readable prtotocol
II: collect and flash AVR firmware images.

===========================================================================
===========================================================================
*/


/*
 **********************************************************************
 **********************************************************************
 Payload, part I: set and get parameters of the target AVR uC
 - "command_avrgetpar(char* par, int* val)" to write "<PAR>?\r\n" to the 
   AVR via the serial line and expect as a reply "<PAR>=<VAL>\r\n". The 
   parameter name <PAR> is given by the argiment par and should be a reasonably
   tame ASCII printable, such as "*[A-Za-z]". It must not contain the letters '?'
   or '='. <VAL> is a 32bit signed integer in the common ASCII encoded decimal 
   notation. 
 - "command_avrsetpar(char* par, int val)" to write "<PAR>=<VAL>\r\n" to the 
   AVR via the serial to set the parameter <PAR> accordingly. As above, 
   <VAR> specifies the actual value as a 32bit signed integer. The target
   AVR replies as in the get-parameter request.

 **********************************************************************
 **********************************************************************
 */

// we only do AVR comms when there actually is an AVR target uC to care of
// (see below for the simulation of a target uC otherwise)
#ifdef AVR_PRESENT


// mutex for AVR UART communications (so far, there are no competing tasks
// and "node_read_task" exclusively uses the UART -- this however may change)
SemaphoreHandle_t g_avruart_mutex;

// line buffer size
#define MAX_LINE 128

// UART helper: clear line buffers
void avrclearln(void) {
  uart_wait_tx_done(UART_NUM_1, 1000 / portTICK_PERIOD_MS);
  uart_flush_input(UART_NUM_1);
}        

// UART helper: write line
// this will append line termination aka "\r\n"
int avrwriteln(char* msg) {
    size_t len=strlen(msg);
    if(uart_write_bytes(UART_NUM_1, msg, len)!=len) {
        MDF_LOGI("avrwriteln: uart error: buffer overflow");
	return MDF_FAIL;
    }
    if(uart_write_bytes(UART_NUM_1, "\r\n", 2)!=2) {
        MDF_LOGI("avrwriteln: uart error: buffer overflow");
	return MDF_FAIL;
    }
    uart_wait_tx_done(UART_NUM_1, len+2 / portTICK_PERIOD_MS);
    MDF_LOGI("avrwriteln: ok \"%s\"", msg);
    return MDF_OK;
}

// UART helper: read line
// this will read until the next "\n"; before returning the result,
// the terminating "\n" is removed as well as an immediately preceeding
// "\r"; a timeout must be specified in ms; the result needs to be
// allocated by the caller and must be capable to hold MAX_LINE bytes.
int avrreadln(char* str, int timeout) {
    size_t pos=0;
    int cnt;
    while(1) {
        cnt=uart_read_bytes(UART_NUM_1, (unsigned char*) str+pos, 1, timeout/portTICK_PERIOD_MS);
        if(cnt==0) {
	    str[0]=0;
            MDF_LOGI("avrreadln: uart error: time out (#%d)",pos);	  
	    return MDF_FAIL;
	}
        if(str[pos]=='\n') break;
        ++pos;
        if(pos>=MAX_LINE) {
	    str[0]=0;
	    MDF_LOGI("avrreadln: uart error: buffer overflow");	  
            return MDF_FAIL;
	}
    }
    str[pos]=0;
    if(pos>0) if(str[pos-1]=='\r') str[pos-1]=0;
    MDF_LOGI("avrreadln: ok \"%s\"",str);
    return MDF_OK;
}    

// Set a parameter on the target AVR
// return MDF_OK or MDF_FAIL
int command_avrsetpar(char* par, int val) {
    char avrstr[MAX_LINE];
    int rval;
    size_t len;
    char* pstr;
    mdf_err_t ret          = MDF_FAIL;

    // say hello and check range (our target AVR uses signed 16bit integers)
    MDF_LOGI("avrsetpar: %s=%d", par, val);
    if((val< -0x7fff) || (val> 0x7fff)) {
      MDF_LOGI("avrsetpar: value out if range");
      return ret;
    }       
    // take mutex
    if(xSemaphoreTake(g_avruart_mutex, 1000/portTICK_PERIOD_MS) != pdTRUE)
       return ret;
    // assemble command string e.g. "power=20" and erite to AVR
    if(snprintf(avrstr,MAX_LINE-2,"%s=%d",par,val)>=MAX_LINE-2) goto FREE_MEM;
    // write to AVR, await reply (1000ms timeout, tune this down)
    avrclearln();
    if(avrwriteln(avrstr)!=MDF_OK) goto FREE_MEM;
    if(avrreadln(avrstr,1000)!=MDF_OK) goto FREE_MEM;
    //parse result
    MDF_LOGI("avrsetpar: parsing \"%s\"",avrstr);
    len=strlen(avrstr);
    pstr=strchr(avrstr,'=');
    if(!pstr) {
      MDF_LOGI("avrsetpar: parse error (no '=')");
      goto FREE_MEM;
    }  
    if(pstr+1>=avrstr+len) {
      MDF_LOGI("avrsetpar: parse error (no value, string too short)");
      goto FREE_MEM;
    }  
    int cnt=sscanf(pstr+1,"%d",&rval);
    if(cnt!=1) {
      MDF_LOGI("avrsetpar: parse error (no value, failed to parse int)");
      goto FREE_MEM;
    }
    // validate parameter value
    if(val!=rval) {
      MDF_LOGI("avrsetpar: failed validation)");
      goto FREE_MEM;
    }    
    // done, give away mutex
    ret = MDF_OK;

FREE_MEM:    
    xSemaphoreGive(g_avruart_mutex);
    return ret;
} 
    

// sync AVR systime
// this is just like "command_avrsetpar("time",systime())"
// except if we actually invoked that function we would
// suffer inaccuracy due to task switching, mutexes etc.
// return MDF_OK or MDF_FAIL
int command_avrsettime(void) {
    char *avrstr           =NULL;
    mdf_err_t ret          = MDF_FAIL;

    MDF_LOGI("avrsettime; entry at %d",systime());
    if(xSemaphoreTake(g_avruart_mutex, 1000/portTICK_PERIOD_MS) != pdTRUE)
       return ret;
    avrclearln();
    asprintf(&avrstr,"time=%d",systime() % 30000); // our avr target has a 30sec rollover  
    ret= avrwriteln(avrstr);
    MDF_LOGI("avrsettime: set to %d",systime());
    MDF_FREE(avrstr);
    xSemaphoreGive(g_avruart_mutex);
    return ret;
} 

// get a target parameter
// return MDF_OK or MDF_FAIL
int command_avrgetpar(char* par, int* val) {
    mdf_err_t ret          = MDF_FAIL;
    char avrstr[MAX_LINE];
    char* pstr;
    size_t len;

    // say hello and take mutex
    MDF_LOGI("avrgetpar: %s?", par);
    if(xSemaphoreTake(g_avruart_mutex, 1000/portTICK_PERIOD_MS) != pdTRUE)
        return ret;
    // assemble command string e.g. "power?"
    len=strlen(par);
    if(len+1>=MAX_LINE) return MDF_FAIL;
    strcpy(avrstr,par);
    avrstr[len]='?';
    avrstr[len+1]=0;
    // send to AVR, await reply (1000ms timeout, tune this down)
    avrclearln();
    if(avrwriteln(avrstr)!=MDF_OK) goto FREE_MEM;
    if(avrreadln(avrstr,1000)!=MDF_OK) goto FREE_MEM;
    //parse result
    MDF_LOGI("avrgetpar: parsing \"%s\"",avrstr);
    len=strlen(avrstr);
    pstr=strchr(avrstr,'=');
    if(!pstr) {
      MDF_LOGI("avrgetpar: parse error (no '=')");
      goto FREE_MEM;
    }  
    if(pstr+1>=avrstr+len) {
      MDF_LOGI("avrgetpar: parse error (no val a)");
      goto FREE_MEM;
    }  
    int cnt=sscanf(pstr+1,"%d",val);
    if(cnt!=1) {
      MDF_LOGI("avrgetpar: parse error (no val b)");
      goto FREE_MEM;
    }  
    // done, give away mutex
    ret = MDF_OK;
FREE_MEM:    
    xSemaphoreGive(g_avruart_mutex);
    return ret;
}

// get entire target state
int command_avrgetstate(void) {
  // TODO: get all relevant AVR state parameters to local copy
  return MDF_OK;
}  


#endif //end: AVR_PRESENT

/*
 **********************************************************************
 **********************************************************************
 Payload, part I, variant b: if no target uC is present, we fake comms
 by simulation; i.e., we write control parameters to our global copy, we
 read state parameters from our local copy and we mimique a charging ev
 **********************************************************************
 **********************************************************************
*/

#ifndef AVR_PRESENT

// name to target uC parameter conversion (for simulation only)
typedef struct {
    char* parstr;              // parameter name    
    const int32_t* pargetaddr; // address in memory for reading aka state
    int32_t* parsetaddr;       // address in memory for writing aka command
} tpartable_t;

// name to target uC table (NULL-terminated, for simulation only)
const tpartable_t g_tpartable[]={
    {"ver",     &g_tstate_version,   NULL},
    {"blinks",  NULL,                &g_tcontrol_blinks},
    {"maxcur",  &g_tstate_maxcur,    &g_tcontrol_maxcur},
    {"phases",  &g_tstate_phases,    &g_tcontrol_phases},
    {"ccss",    &g_tstate_ccss,      NULL},
    {"cur1",    &g_tstate_cur1,      NULL},
    {"cur2",    &g_tstate_cur2,      NULL},
    {"cur3",    &g_tstate_cur3,      NULL},
    { NULL, NULL, NULL}
};

// dummies for target uC serial comms: set a target uC control value to our local copy
int command_avrsetpar(const char* p, int v)    {
    // figure table entry
    const tpartable_t* ptab=g_tpartable;
    while(ptab->parstr!=NULL) {
        if(!strcmp(ptab->parstr,p)) break;
        ++ptab;
    }
    // unknown parameter
    if(ptab->parstr==NULL) return MDF_FAIL;
    // no control parameter
    if(ptab->parsetaddr==NULL) return MDF_FAIL;
    // set the control
    *(ptab->parsetaddr)=v;
    return MDF_OK;
};

// dummies for target uC serial comms: get a target uC state value from local copy
int command_avrgetpar(const char* p, int* v)   {
    // figure table entry
    const tpartable_t* ptab=g_tpartable;
    while(ptab->parstr!=NULL) {
        if(!strcmp(ptab->parstr,p)) break;
        ++ptab;
    }
    // unknown parameter
    if(ptab->parstr==NULL) return MDF_FAIL;
    // no state parameter
    if(ptab->pargetaddr==NULL) return MDF_FAIL;
    // get the state
    *v=*(ptab->pargetaddr);
    return MDF_OK;
};

// dummies for target uC serial comms: silently ignore set target uC time
int command_avrsettime(void) {
    return MDF_OK;
};

// dummies for target uC serial comms: silently ignore get target uC state
int command_avrgetstate(void) {
    return MDF_OK;
}  



// This task is called at a 1000ms period and is meant to simulate EV charging.
// It assumes that fake hardware will set/clear the local copy of the CCS state to "button pressed"
// by some operator interaction (e.g. M5Stick via button B). The task will directly write
// on relevant components of the local copy of the target uC state, which are in turn forwarded
// via the periodical heartbeat.
static void simulate_target_timercb(void *timer) {
    // pseudo globals for simulation state
    static int ccss=0;
    static int level=0;
    static int cmaxcur=0;
    static int bphases=0;
    static int expire=0;
    static const int capacity=1000;
    // have fake version number (reads "v0.0" nor "none" to indicate simulation)
    g_tstate_version=0;
    // allocated maxcur is set by g_tcontrol_maxcur and expires after 10sec
    if(g_tcontrol_maxcur >= 0) {
        cmaxcur= g_tcontrol_maxcur;
        g_tcontrol_maxcur=-1;
        expire=10; 
    }  
    if(expire>0) {
        expire--;
    }
    if(expire==0) {
        cmaxcur=0;
    }
    g_tstate_maxcur=cmaxcur;
    // copy and decode phase from control data
    if(g_tstate_phases!=g_tcontrol_phases) {
        bphases=0;
        int dphases=g_tcontrol_phases;
        while(dphases>0) {
            int lsd=dphases % 10;
            if(lsd>0) bphases |= (0x1 << (lsd-1));
            dphases = dphases /10;
        }  
        g_tstate_phases=g_tcontrol_phases;
    }  
    // CCS state 0: wait for operator button (mimiqued e.g. by M5Stick button A)
    if(ccss==0) {
      g_tstate_cur1=0;
      g_tstate_cur2=0;
      g_tstate_cur3=0;   
      if(g_tstate_ccss>0) ccss=1;
    }
    // CCS state 1: wait for car (ignored in simulation)
    if(ccss==1) {
      ccss=2;
    }  
    // CCS state 2: wait for load allocation
    if(ccss==2) {
      if((cmaxcur>50) && (bphases!=0)) {
        level=0;
        ccss=3;
      }  
    }
    // CCS state 3: run primitive PT1-style charging curves
    if(ccss==3) {
        int cur = 320.0 * (capacity - level) / capacity; // max 32A
        if(cur>cmaxcur) cur=cmaxcur;
        if(cur<50) cur=0; // min 5A
        g_tstate_cur1= bphases & 0x01 ? cur : 0;
        g_tstate_cur2= bphases & 0x02 ? cur : 0;
        g_tstate_cur3= bphases & 0x04 ? cur : 0;
        level+= cur*0.1;  // scale timing 0.1 <> 40sec for one charge
        if(cur==0) ccss=0;
        if(bphases==0) ccss=0;
    }
    // sense charge abort by operator (mimiqued e.g. by M5Stick button A)
    if((g_tstate_ccss==0) && (ccss!=0)) {
        ccss=0;
    }  
    // forward CCS state (one extra digit for compatibility with CTRL22 target firmware)
    g_tstate_ccss=10*ccss;
}





#endif // end: AVR not present



/*
 **********************************************************************
 **********************************************************************
 Payload, part II: target AVR OTA firmware update

- AVR firmware download is organised in three stages with 
    command_avrota_reset(void) 
    command_avrota_receive(void) 
    command_avrota_flash(void) 
  for transitions. On "reset", we clear/cancel any firmware download an turn to
  normal operation. On "receive", we clear all buffers on the ESP side and expect
  form now on to receive the AVR firmware in packages of moderate size (128bytes).
  On "flash" we use the Optiboot protocol (an STK500 subset) to flash the AVR uC
  via the serial line. 
- "command_avrimage(addr, data, cnt) implements receiving a chunk of firmware data
  and buffering it for later use; this is only considered legal when in mode "receive".
- various UART helper functions implement the Optiboot protocol, incl. avrflash()
  as the overall wrapper.
}  

 **********************************************************************
 **********************************************************************
 */

// anything we provide here (and use later) needs a dummy def at the end of this section
#ifdef AVR_PRESENT

// data partition for AVR firmware (configured for max 64Kb, see partitiontable)
static const esp_partition_t* g_avrimg_part=NULL;

// record of received chunks of the image (one bit per chunk)
#define AVR_IMG_OK_CNT (AVR_IMG_CNT/AVR_OPT_CNT/8 + 1)
unsigned char g_avrimg_ok[AVR_IMG_OK_CNT];
int g_avrimg_cnt=0;

// command buffer size
#define MAX_OPTCMD (AVR_OPT_CNT+10)   // one page data plus command parameters

// global optiboot command buffer
unsigned char g_avroptbuf[MAX_OPTCMD];

// UART helper: talk to optiboot as defined in the STK500 reference by ATMEL
// - the command code and the parameters are in the global command buffer,
//   so we only need to specify the count (cnt) and the expected count of the reply (rcnt)
// - after the command, we append an Sync_EOP (0x20) and expect as first reply a Sync_Resp
//   (0x14); otherwise we have a sync error
// - return MDF_OK or MDF_FAIL
int avroptcmd(int cnt, int rcnt) {
    // flush uart
    uart_flush_input(UART_NUM_1);
    // add the Sync_EOP
    g_avroptbuf[cnt]=0x20;
    // send command
    if(uart_write_bytes(UART_NUM_1,  (char*) g_avroptbuf, cnt+1)!=cnt+1) {
        MDF_LOGI("avroptcmd: uart error: buffer overflow");
	return MDF_FAIL;
    }
    uart_wait_tx_done(UART_NUM_1, cnt+2 / portTICK_PERIOD_MS);
    MDF_LOGD("avroptcmd: sent command 0x%02x ... (#%d)",g_avroptbuf[0],cnt);
    //ESP_LOG_BUFFER_HEX("XMEGA OPTI CMD", g_avroptbuf,cnt+1);
    // await reply
    int pos=0;
    while(rcnt>=0) {
        cnt=uart_read_bytes(UART_NUM_1, g_avroptbuf+pos, rcnt+1, 100/portTICK_PERIOD_MS);
        if(cnt==0) {
            MDF_LOGI("avroptcmd: uart error: time out (#%d)",pos);	  
	    return MDF_FAIL;
	}
	pos+=cnt;
	rcnt-=cnt;
    }
    if(pos<=1) g_avroptbuf[1]=0x00;
    MDF_LOGD("avroptcmd: received reply 0x%02x ... (#%d)",g_avroptbuf[1],pos-1);
    if(g_avroptbuf[0]!=0x14) {
        MDF_LOGI("avroptcmd: uart error: failed sync");
	return MDF_FAIL;
    }      
    return MDF_OK;
}

// Optiboot: sync request
int avroptsync(void) {
    g_avroptbuf[0]=0x30; // op-code "sync request"
    avroptcmd(1,1);
    return g_avroptbuf[1]==0x10 ? MDF_OK : MDF_FAIL;
}

// Optiboot: ask for STK version
int avroptversion(void) {
    int minor;
    int major;
    g_avroptbuf[0]=0x41;                                   // opcode "read parameter"
    g_avroptbuf[1]=0x81;                                   // major version
    avroptcmd(2,2);
    major=g_avroptbuf[1];
    if(g_avroptbuf[2]!=0x10) return MDF_FAIL;
    g_avroptbuf[0]=0x41;                                   // opcode "read parameter"
    g_avroptbuf[1]=0x82;                                   // minor version
    avroptcmd(2,2);
    minor=g_avroptbuf[1];
    if(g_avroptbuf[2]!=0x10) return MDF_FAIL;
    MDF_LOGD("avroptversion: found optboot STK500  v%d.%d", major,minor);
    return MDF_OK;
}
     

// Optiboot: load word address (or byte address for mega-core series)
int avroptaddr(unsigned int avraddr) {
    g_avroptbuf[0]=0x55;                                   // opcode "set address"
#ifdef AVR_OPT_TADDR
    avraddr+=AVR_OPT_TADDR;  // add offset to application image (mega-core series)
#endif    
#ifndef AVR_OPT_BADDR
    avraddr=avraddr/2;       // transform byte address to word address (pre mega-core series)
#endif    
    g_avroptbuf[1]= (unsigned char) (avraddr & 0xff);  // low byte of word(!) address
    g_avroptbuf[2]= (unsigned char) (avraddr  >> 8);    // high byte of work address
    avroptcmd(3,1);
    return g_avroptbuf[1]==0x10 ? MDF_OK : MDF_FAIL;
}

// Optiboot: write flash page 
int avroptwrite(unsigned int avraddr) {
    mdf_err_t ret          = MDF_FAIL;
    g_avroptbuf[0]=0x64;                // opcode "write page"
    g_avroptbuf[1]= AVR_OPT_CNT >> 8;   // high byte of byte count
    g_avroptbuf[2]= AVR_OPT_CNT & 0xff; // low byte of byte count
    g_avroptbuf[3]= 0x46;               // destination is flash
    ret=esp_partition_read(g_avrimg_part, avraddr, g_avroptbuf+4, AVR_OPT_CNT); 
    if(ret != MDF_OK) {  
        MDF_LOGD("avroptflash: failed reading from ESP flash: <%s> esp_partition_read", mdf_err_to_name(ret));
        return MDF_FAIL;
    }
    avroptcmd(AVR_OPT_CNT+4,1);
    return g_avroptbuf[1]==0x10 ? MDF_OK : MDF_FAIL;
}

// Optiboot: verify flash page
int avroptverify(unsigned int avraddr) {

    mdf_err_t ret          = MDF_FAIL;
    unsigned char vbuf[AVR_OPT_CNT];
    int pos;

    g_avroptbuf[0]=0x74;                // opcode "read page"
    g_avroptbuf[1]= AVR_OPT_CNT >> 8;   // hight byte of byte count
    g_avroptbuf[2]= AVR_OPT_CNT & 0xff; // low byte of byte count
    g_avroptbuf[3]= 0x46;               // source flash
    avroptcmd(4,AVR_OPT_CNT+1);
    if(g_avroptbuf[AVR_OPT_CNT+1]!=0x10)
        return MDF_FAIL;
    ret=esp_partition_read(g_avrimg_part, avraddr,vbuf, AVR_OPT_CNT); 
    if(ret != MDF_OK) {  
        MDF_LOGD("avroptverify: failed reading from ESP flash: <%s> esp_partition_read", mdf_err_to_name(ret));
        return MDF_FAIL;
    }
    for(pos=0; pos<AVR_OPT_CNT; ++pos)
        if(vbuf[pos]!=g_avroptbuf[pos+1]) break;
    if(pos<AVR_OPT_CNT) {
        MDF_LOGD("avroptverify: error at byte address 0x%04x (read back 0x%02x vs programmed 0x%02x)",
		 avraddr + pos,g_avroptbuf[pos+1],vbuf[pos]);
	//ESP_LOG_BUFFER_HEX("XMEGA", g_avroptbuf+1,AVR_OPT_CNT);
        return MDF_FAIL;
    }	
    return MDF_OK;
}


// Optiboot: quit to application
int avroptquit(void) {
    g_avroptbuf[0]=0x51; // opcode "exit to application programm"
    avroptcmd(1,1);
    return g_avroptbuf[1]==0x10 ? MDF_OK : MDF_FAIL;
}


// reset/run avr
void avrreset(int halt) {
    if(halt) {
        if(AVR_RST_ACTIVE) {
            gpio_set_level(AVR_RST_GPIO,0);
        } else {   
            gpio_set_direction(AVR_RST_GPIO, GPIO_MODE_OUTPUT);
            gpio_set_level(AVR_RST_GPIO,0);
	}	    
    } else {
        if(AVR_RST_ACTIVE) {
            gpio_set_level(AVR_RST_GPIO,1);
        } else {   
            gpio_set_level(AVR_RST_GPIO,1);    
            gpio_set_direction(AVR_RST_GPIO, GPIO_MODE_INPUT);
	}
    }	
}

// Optiboot: overall wrapper to flash AVR 
// return MDF_OK or MDF_FAIL
int avrflash(void) {

    mdf_err_t ret          = MDF_FAIL;
    
    // say hello and take mutex
    MDF_LOGI("avrsflash: do flash now");
    if(xSemaphoreTake(g_avruart_mutex, 1000/portTICK_PERIOD_MS) != pdTRUE)
        return ret;
    // reset AVR target
    avrreset(1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    avrreset(0);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    // clear serial buffers
    avrclearln();    
    // sync with optiboot
    int fail=20;
    for(; fail>0; fail--) {
        if(avroptsync()==MDF_OK) break;
	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    if(fail==0) {
        MDF_LOGD("avrflash: failed to sync with optiboot");
        goto FREE_MEM;
    }
    // verify version
    if(avroptversion()!=MDF_OK) {
        MDF_LOGI("avrflash: failed verify STK500 version");
	goto FREE_MEM;
    }
    // flash
    for(unsigned int addr=0; addr<g_avrimg_cnt; addr+=AVR_OPT_CNT) {
       MDF_LOGI("avrflash: flashing at byte address 0x%04x",addr);
       for(; fail>0; fail--) {
            if(avroptaddr(addr)==MDF_OK) break;
	    vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if(fail==0) {
            MDF_LOGI("avrflash: failed to set byte address 0x%04x",addr);
            goto FREE_MEM;
        }
        for(; fail>0; fail--) {
            if(avroptwrite(addr)==MDF_OK) break;
    	    vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if(fail==0) {
            MDF_LOGD("avrflash: failed to flash at byte address 0x%04x",addr);
            goto FREE_MEM;
        }
    }
    // verify
    for(unsigned int addr=0; addr< g_avrimg_cnt; addr+=AVR_OPT_CNT) {
        MDF_LOGI("avrflash: verifying at byte address 0x%04x",addr);
        for(; fail>0; fail--) {
            if(avroptaddr(addr)==MDF_OK) break;
	    vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if(fail==0) {
            MDF_LOGD("avrflash: failed to set byte address 0x%04x",addr);
            goto FREE_MEM;
        }
        for(; fail>0; fail--) {
            if(avroptverify(addr)==MDF_OK) break;
    	    vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if(fail==0) {
            MDF_LOGD("avrflash: failed to verify at byte address 0x%04x",addr);
            goto FREE_MEM;
        }
    }
    // quit optiboot
    avroptquit();
    ret=MDF_OK;
FREE_MEM:
    xSemaphoreGive(g_avruart_mutex);
    return ret;
}

// state of avr firmware download (0:normal operation, 1:reveice image data, 2: do flash)
int g_avrstate=0;

// set state 0: initial state
int command_avrota_reset(void) {
    MDF_LOGI("command_avrota_reset()");
    g_avrstate=0;
    return MDF_OK;
}  

// set state 1: ready to receive AVR firmware packages
int command_avrota_receive(void) {
    MDF_LOGI("command_avrota_receive()");
    mdf_err_t ret          = MDF_FAIL;
    // erase flash before writing first packet
    ret=esp_partition_erase_range(g_avrimg_part, 0, g_avrimg_part->size);
    if(ret != MDF_OK) {
        MDF_LOGD("avrimage: failed to erase flash: <%s> esp_partition_erase_range", mdf_err_to_name(ret));
	return MDF_FAIL;
    }
    // clear records
    memset(g_avrimg_ok,0x00,AVR_IMG_OK_CNT);
    g_avrimg_cnt=0;
    // set state
    g_avrstate=1;
    return MDF_OK;
}  

// set state 2: do flash AVR (argument is the image size as byte count for validation)
int command_avrota_flash(int avrimgcnt) {
    MDF_LOGI("command_avrota_flash()");
    int avrpage;
    // check for complete image 
    if(g_avrstate!=1) {
        MDF_LOGI("command_avrota_flash: not in state \"receive\"");
        return MDF_FAIL;
    }
    if(g_avrimg_cnt!=avrimgcnt) {
        MDF_LOGI("command_avrota_flash: firmware image incomplete (to little byte count)");
        return MDF_FAIL;
    }
    for(avrpage=0; avrpage*AVR_OPT_CNT<g_avrimg_cnt; ++avrpage) {
	MDF_LOGI("command_avrota_flash: test page %d)",avrpage);
	if((g_avrimg_ok[avrpage / 8] & (0x01 << (avrpage % 8)))!=0x00) continue;
	MDF_LOGI("command_avrota_flash: firmware image incomplete (missing page %d)",avrpage);
        return MDF_FAIL;    
    }
    // do programm
    g_avrstate=2;
    if(avrflash()!=MDF_OK) {
        return MDF_FAIL;
    }	
    command_avrsettime();
    // back to reset
    g_avrstate=0;
    return MDF_OK;
}    
      


// receive chunk of AVR firmware as Base64 ASCII string
// return MDF_OK or MDF_FAIL
int command_avrimage(int avraddr, char* avrdata64, uint32_t avrcrc) {

    mdf_err_t ret          = MDF_FAIL;
    size_t cnt, len, dlen;
    unsigned char* avrdata = NULL;
    uint32_t crc;
    
    MDF_LOGI("avrimage : 0x%04x", avraddr);
    // sanity check
    if(g_avrstate!=1) {
        MDF_LOGI("avrimage : not in state \"receive\"");
        goto FREE_MEM;
    }
    if(avraddr> 48*1024) {
        MDF_LOGI("avrimage : address out of range");
        goto FREE_MEM;
    }
    len=strlen(avrdata64);
    if(len *3 > 128 *4 + 4) {
        MDF_LOGI("avrimage : data string too long (#%d)", len);
        goto FREE_MEM;
    }
    // convert to binary 
    dlen=(len*3)/4 + 1;
    avrdata=MDF_MALLOC(dlen);
    if(mbedtls_base64_decode((unsigned char*) avrdata, dlen, &cnt, (unsigned char*) avrdata64, len)!=0) {
        MDF_LOGI("avrimage : failed to decode data string");
        goto FREE_MEM;
    }
    // avrdata = base64_decode((unsigned char*)avrdata64, len, &cnt); // pre 107
    if(avrdata==NULL) {
        MDF_LOGI("avrimage : failed to decode data string");
        goto FREE_MEM;
    }
    // write to flash 
    ret = esp_partition_write(g_avrimg_part, avraddr, avrdata, cnt);
    if(ret != MDF_OK) {
        MDF_LOGD("avrimage: failed to write to flash: <%s> esp_partition_write", mdf_err_to_name(ret));
        goto FREE_MEM;
    }
    // read back from flash
    ret = esp_partition_read(g_avrimg_part, avraddr, avrdata, cnt);
    if(ret != MDF_OK) {  
        MDF_LOGD("avrimage: failed to read back from flash: <%s> esp_partition_write", mdf_err_to_name(ret));
        goto FREE_MEM;
    }
    // verify crc32
    crc=crc32_le(0x00000000, avrdata, cnt);
    if(avrcrc != crc)  {
        MDF_LOGI("avrimage : crc32 mismatch 0x%08x vs 0x%08x",avrcrc,crc);
	ret=MDF_FAIL;
        goto FREE_MEM;
    }
    // record success (trust in the fact, that chunks arrive with full AVR_OPT_CNT length, except for the last page)
    int avrpage=avraddr/AVR_OPT_CNT;
    g_avrimg_ok[avrpage / 8] |= (0x01 << (avrpage % 8));
    if(g_avrimg_cnt < avraddr+cnt) g_avrimg_cnt = avraddr+cnt;
    ret=MDF_OK;
FREE_MEM:
    if(avrdata) MDF_FREE(avrdata);
    return ret;
};


// no AVR to care about --- then provide dummies
#else

// all dummies to match this sectiom
int command_avrota_receive(void)             { return MDF_FAIL;} ;
int command_avrota_flash(int cnt)            { return MDF_FAIL;} ;
int command_avrota_reset (void )             { return MDF_FAIL;} ;
int command_avrimage(int a, char* i, int crc)  { return MDF_FAIL;} ;
void avrreset(int halt) {} ;
void avrclearln(void) {} ;

#endif



/*
===========================================================================
===========================================================================
The remainder is just about the mesh ourselfs ...
- install firmware updates
- connect to host via plain TCP socket and dispatch commands
- connect to mqtt broker
===========================================================================
===========================================================================
*/


/*
 **********************************************************************
 **********************************************************************
 Mupgrade 
 - largely based on ESP-MDF example code ("mupgrade_example.c")
 - use "g_upgrade_version" and "g_upgrade_board" to specify the firmware
   name (e.g. "demesh v3.4 (m5stick)") and filename on the server
   (e.g. "demesh_m5stick_3_4.bin")
 - the firmware name is used by the individual nodes to decide whether 
   or not to accept the update; see "mesh_event_cb" in this regard
 **********************************************************************
 **********************************************************************
 */



static void upgrade_task(void* ard)
{
    mdf_err_t ret       = MDF_OK;
    uint8_t *data       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    char *name          = NULL;
    char *url           = NULL;
    size_t total_size   = 0;
    int start_time      = 0;
    mupgrade_result_t upgrade_result = {0};
    mwifi_data_type_t data_type = {.communicate = MWIFI_COMMUNICATE_MULTICAST};

 
    MDF_LOGI("Upgrade task is running, target set to version %s on board %s",g_upgrade_version,g_upgrade_board);
 
    // upgrade all devices
    uint8_t dst_addr[][MWIFI_ADDR_LEN] = {MWIFI_ADDR_ANY};

    // set up firmware name
    asprintf(&name,"%s v%s (%s)",CONFIG_FIRMWARE_BASENAME,g_upgrade_version,g_upgrade_board);

    // set up firmware url
    asprintf(&url,"http://%s:%d/%s_%s_%s.bin",CONFIG_FIRMWARE_SERVER_IP,CONFIG_FIRMWARE_SERVER_PORT,
       CONFIG_FIRMWARE_BASENAME,g_upgrade_board,g_upgrade_version);
    for(int i=strlen(url)-5; (i>0) && (url[i]!='/'); --i) 
      if(url[i]=='.') url[i]='_';
              
    // set up http client
    MDF_LOGI("Upgrade: open HTTP socket for %s", url);
    esp_http_client_config_t config = {
	 .url            = url,
	 .transport_type = HTTP_TRANSPORT_UNKNOWN,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 1. check for firmware http server
    MDF_LOGI("Upgrade: Open HTTP connection for %s", url);
    start_time = xTaskGetTickCount();
    for(int i=10; i>0  ; --i) {
        if (!esp_mesh_is_root()) goto FREE_MEM;
 	vTaskDelay(1000 / portTICK_PERIOD_MS);
        ret = esp_http_client_open(client, 0);
	if( ret == MDF_OK ) break;
    }	
    MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> failed to open HTTP connection", mdf_err_to_name(ret));
    g_upgrade_stage=1;
      
    // 2. obtain firmware header
    total_size = esp_http_client_fetch_headers(client);
    MDF_ERROR_GOTO(total_size <= 0, FREE_MEM, "failed to access firmware (i.e. HTTP file not found)");
    g_upgrade_stage=2;

    // 3. initialize the upgrade status and erase the upgrade partition.
    MDF_LOGI("Upgrade: initialise mupgrade %s", url);
    ret = mupgrade_firmware_init(name, total_size);
    MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> Initialize mupgrade", mdf_err_to_name(ret));
    g_upgrade_stage=3;

    // 4. read firmware from the server and write it to the flash of the root node
    for (ssize_t size = 0, recv_size = 0; recv_size < total_size; recv_size += size) {
        if (!esp_mesh_is_root()) goto FREE_MEM;
        size = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
        MDF_ERROR_GOTO(size <= 0, FREE_MEM, "<%s> upgrade task failed to read from HTTP stream", mdf_err_to_name(ret));
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (size > 0) {
	    ret = mupgrade_firmware_download(data, size);
	    MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> upgrade task failed to rite firmware to flash, size: %d, data: %.*s",
			 mdf_err_to_name(ret), size, size, data);
	}  
      }
      MDF_LOGI("Upgrade: download firmware is complete, spent time %ds",
	       (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS / 1000);
      g_upgrade_stage=4;
      

      // 5.  distribute the firmware to all nodees.
      start_time = xTaskGetTickCount();
      if (!esp_mesh_is_root()) goto FREE_MEM;
      ret = mupgrade_firmware_send((uint8_t *)dst_addr, sizeof(dst_addr) / MWIFI_ADDR_LEN, &upgrade_result);
      MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> upgrade task failed to distribute firmware to nodes", mdf_err_to_name(ret));
      g_upgrade_stage=5;
      if (upgrade_result.successed_num == 0) {
        MDF_LOGW("Upgrade: not a sinlge device recieved and accepted the upgrade");
        goto FREE_MEM;
      }
      MDF_LOGI("Upgrdae: firmware sent to relevant devices, spent time %ds",
	       (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS / 1000);
      MDF_LOGI("Upgrade: #%d devices completed,  #%d devices unfinished", upgrade_result.successed_num, upgrade_result.unfinished_num);

      // 6. the root notifies nodes to restart
      const char *restart_str = "{\"cmd\": \"restart\"}";
      ret = mwifi_root_write(upgrade_result.successed_addr, upgrade_result.successed_num,
			     &data_type, restart_str, strlen(restart_str), true);
      MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> upgrade task failed when sending restart command", mdf_err_to_name(ret));
      g_upgrade_stage=6;
      vTaskDelay(20000 / portTICK_PERIOD_MS);  
 
FREE_MEM:
    MDF_FREE(data);
    MDF_FREE(url);
    MDF_FREE(name);
    mupgrade_result_free(&upgrade_result);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    MDF_FREE(g_upgrade_version);
    MDF_FREE(g_upgrade_board);
    g_upgrade_stage=0;
    vTaskDelete(NULL);
}


/*
 **********************************************************************
 **********************************************************************
 Upstream client (root node only, effectively a plain TCP client)
 - the root node is connected  by the upstream server and waits for messages to
   be forwarded to specific node(s)
 - expected messsages are JSON encoded with a "dst":"<DEST_ADDR>" field, where
   <DEST_ADDR> specifies the mac address of the destination node and a
   "cmd":"<CMD>" specifies the RCP to be executed
 - use the special addresses "root" for the root node and "*" for a 
   broadcast; 
 - examples: 
     {"dst":"24:6f:28:22:83:ac","cmd":"status"}
     {"dst":"root","cmd":"upgrade","version":"3.4","board":"m5stick"}}
     {"dst":"*","cmd":"system"}
     {"dst":"d8:a0:1d:54:e4:a4","cmd":"avrsetpar","avrpar":"blinks","avrval":3}
 - the destination address field is removed befor forwarding the 
   message to the respective node.

 **********************************************************************
 **********************************************************************
 */


// read from upstream server
void upstream_read_task(void *arg)
{
    mdf_err_t ret                     = MDF_OK;
    char *data                        = MDF_MALLOC(MWIFI_PAYLOAD_LEN+1);
    size_t size                       = MWIFI_PAYLOAD_LEN;
    uint8_t dst_addr[MWIFI_ADDR_LEN] = {0x0};
    uint8_t any_addr[MWIFI_ADDR_LEN]  = MWIFI_ADDR_ANY;
    uint8_t *dst_addr_p              = NULL;
    mwifi_data_type_t data_type       = {0x0};
    cJSON *json_root                  = NULL;
    cJSON *json_dst                  = NULL;

    MDF_LOGI("Upstream client read task is running");

    while (mwifi_is_connected()) {

        // connect to server 
        if (g_sockfd == -1) {
            MDF_LOGI("Upstream: create a tcp client to connect to server at, ip: %s, port: %d",
		CONFIG_UPSTREAM_SERVER_IP, CONFIG_UPSTREAM_SERVER_PORT);

	    // set server address 
	    g_server_addr.sin_family = AF_INET;
            g_server_addr.sin_port = htons(CONFIG_UPSTREAM_SERVER_PORT);
            g_server_addr.sin_addr.s_addr = inet_addr(CONFIG_UPSTREAM_SERVER_IP);

            // TCP variant
            int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	    if(sockfd>=0) {
                ret = connect(sockfd, (struct sockaddr *)&g_server_addr, sizeof(g_server_addr));
	        if(ret != 0) { 
		    close(sockfd);
		    sockfd=-1;
		}
	    }
	    // UDP variant
	    // sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	    if(sockfd>0) {
	        g_sockfd=sockfd;
                MDF_LOGI("Upstream: connected to server, ip: %s, port: %d",
		   CONFIG_UPSTREAM_SERVER_IP, CONFIG_UPSTREAM_SERVER_PORT);
	    } else {   
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
                MDF_ERROR_CONTINUE(g_sockfd < 0, "<%s> upstream task failed to connect to server", strerror(errno));
	    }
	}  
		  
        // read from server
        memset(data, 0, MWIFI_PAYLOAD_LEN+1);
        // TCP variant
        ret = read(g_sockfd, data, size);
	// UDP variant
        // socklen_t socklen = sizeof(g_server_addr);
        // ret = recvfrom(g_sockfd, data, size, 0, (struct sockaddr *)&g_server_addr, &socklen);
        if (ret <= 0) {
            MDF_LOGW("<%s> read", strerror(errno));
            close(g_sockfd);
            g_sockfd = -1;
            continue;
        }
        MDF_LOGD("Upstream: received message, size: %d, data: %.20s", size, data);


	// parse destination node
        json_root = cJSON_Parse(data);
        MDF_ERROR_CONTINUE(!json_root, "Upstream: JSON parse error (cJSON_Parse)");
        json_dst = cJSON_GetObjectItem(json_root, "dst");
        if (! json_dst ) {
  	    MDF_LOGW("Uptream: cannot forward, no detination address specified");
            cJSON_Delete(json_root); 	    
            continue;
        }
	if ( !strcmp(json_dst->valuestring,"*") ){
  	    MDF_LOGW("Upstream: forwarding broadcast message");
	    dst_addr_p=any_addr;
	} else {   
	if ( !strcmp(json_dst->valuestring,"root") ){
  	    MDF_LOGW("Upstream: forwarding root message");
            esp_wifi_get_mac(ESP_IF_WIFI_STA, dst_addr);	    
	    dst_addr_p=dst_addr;
	} else {
	  MDF_LOGW("Upstream: forwarding to destination node %s",json_dst->valuestring);
  	    uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
            sscanf(json_dst->valuestring, MACSTR,
            mac_data, mac_data + 1, mac_data + 2,
            mac_data + 3, mac_data + 4, mac_data + 5);
            for (int i = 0; i < MWIFI_ADDR_LEN; i++) {
               dst_addr[i] = mac_data[i];
            }
	    dst_addr_p=dst_addr;
	}}    

	// prepare payload (strip destination)
        cJSON_DetachItemViaPointer(json_root,json_dst);
	cJSON_Delete(json_dst);
        char *send_data = cJSON_PrintUnformatted(json_root);

	// send
        MDF_LOGD("Upstream: forward address " MACSTR ", data size %d", MAC2STR(dst_addr_p),strlen(send_data));
        ret = mwifi_write(dst_addr_p, &data_type, send_data, strlen(send_data), true);
        MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> upstream failed to write to mesh", mdf_err_to_name(ret));

    FREE_MEM:
        MDF_FREE(send_data);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("Upstream: client read task is exit");

    close(g_sockfd);
    g_sockfd = -1;
    MDF_FREE(data);
    vTaskDelete(NULL);
}



/*
 **********************************************************************
 **********************************************************************
 Root read: 

 - forward firmware upgrade related messages to ESP-MDF mupgrade handler
 - forward node messages to upstream server
   messages should be JSON encoded and include a "src" record and an "mtype" record;
   the latter indicates whether the message is an acknowledgements to commands or not;
   examples
     {"src":"24:6f:28:22:83:ac","mtype":"status", "parent":"d8:a0:1d:55:a7:11","rssi":-34, [..aso..]}
     {"src":"24:6f:28:22:83:ac","mtype":"heartbeat","rssi":-40, [..aso..]}
 - forward heartbeat, system and status messages to MQTT broker

 **********************************************************************
 **********************************************************************
 */


static void root_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0};

    char* rtopic=NULL;
    char* rdata=NULL;
    int rdsize=0;
    char srcdev[12+1] ={0};

    cJSON *json_root                  = NULL;
    cJSON *json_src                   = NULL;
    cJSON *json_dev                   = NULL;
    cJSON *json_mtype                 = NULL;
    char hbmsg =0;
    char akmsg =0;
    
  
    MDF_LOGI("Root read task is running");

    while (mwifi_is_connected()) {
        // do the read
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> root read failed (mwifi_root_recv)", mdf_err_to_name(ret));

	// dispatch: firmware upgrade data
        if (data_type.upgrade) { 
            ret = mupgrade_root_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> root read failed (mupgrade_root_handle)", mdf_err_to_name(ret));
	    continue;
        }

	// strip "\r\n" .. purely cosmetic
	if( (data[size-1] == '\n') || (data[size-1] == '\r') ) {
	    data[size-1]=0;
	    size--;
  	    if( (data[size-1] == '\n') || (data[size-1] == '\r') ) {
	        data[size-1]=0;
  	        size--;
	    }	
	}
	  
	// everything else should be a JSON string
        MDF_LOGI("Root read: received data from " MACSTR ", size: %d, data: %s",
                    MAC2STR(src_addr), size, data);

        // dispatch: forward to tcp upstream server
        if (g_sockfd > 0) {
  	    ret = write(g_sockfd, data, size);
            MDF_ERROR_CONTINUE(ret <= 0, "<%s> root read failed when forwarding message to upstream server", strerror(errno));
	}

	// dispatch: forward to mqtt broker
        if(g_mqtt_client) {
	  
  	    // partially parse JSON string
            json_root = cJSON_Parse(data);
            MDF_ERROR_GOTO(!json_root, MQTT_FAIL, "Root read: mqtt forward: parse error (cJSON_Parse)");
            json_src = cJSON_GetObjectItem(json_root, "src");
            MDF_ERROR_GOTO(!json_src, MQTT_FAIL, "Root read: mqtt forward: parse error: no src");            
            json_mtype = cJSON_GetObjectItem(json_root, "mtype");
            MDF_ERROR_GOTO(!json_mtype, MQTT_FAIL, "Root read: mqtt forward: parse error: no mtype");            
            MDF_ERROR_GOTO(!cJSON_IsString(json_mtype), MQTT_FAIL, "Root read: mqtt forward: invalid mtype a");

            // only forward heartbeat, system, and status messages.
	    hbmsg = !strcmp(json_mtype->valuestring,"heartbeat");
	    akmsg = !strcmp(json_mtype->valuestring,"system");
	    akmsg |= !strcmp(json_mtype->valuestring,"status");  
	    akmsg |= !strcmp(json_mtype->valuestring,"avrgetpar");  
	    akmsg |= !strcmp(json_mtype->valuestring,"avrsetpar");  
	    if(hbmsg || akmsg) {
	   
		// prepare payload: remove mtype from heartbeat message
	        /*
	        if(hbmsg) {
	          cJSON_DetachItemViaPointer(json_root,json_mtype);
	          cJSON_Delete(json_mtype);
		} 
                */ 

		// prepare payload: replace "mac-style src" by "mqtt-style dev"
	        snprintf(srcdev,13,NOCMACSTR,NOCMAC2STR(src_addr));
		json_dev=cJSON_CreateStringReference(srcdev);
		cJSON_AddItemToObject(json_root,"dev",json_dev);
	        cJSON_DetachItemViaPointer(json_root,json_dev);
	        cJSON_ReplaceItemViaPointer(json_root,json_src,json_dev);

                // convert message to string
                rdata = cJSON_PrintUnformatted(json_root);
	        rdsize=strlen(rdata);
		
	        // publish (heartbeat messages have topic 'heartbeat', anything else has topic 'achnowledge')
		if(hbmsg)
  	          asprintf(&rtopic, "/" CONFIG_MESH_ID "/" NOCMACSTR "/heartbeat" , NOCMAC2STR(src_addr));
		else
  	          asprintf(&rtopic, "/" CONFIG_MESH_ID "/" NOCMACSTR "/acknowledge" , NOCMAC2STR(src_addr));
	        if(esp_mqtt_client_publish(g_mqtt_client, rtopic, rdata, rdsize, 0, 0)!=ESP_OK)
	  	    MDF_LOGW("publishing hearbeat to mqtt broaker failed");
		else      
		    MDF_LOGW("publishing hearbeat to mqtt broaker succeeded");
	    }	


   	}	
    MQTT_FAIL:



    //FREE_MEM:

   	// free buffers
        if(json_root) cJSON_Delete(json_root);
	json_root=NULL;
        if(rdata) MDF_FREE(rdata);
        rdata=NULL;
        if(rtopic) MDF_FREE(rtopic);
        rtopic=NULL;
	
    }

    MDF_LOGW("Root read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}



/*
 **********************************************************************
 **********************************************************************
 * publish/subscribe heartbeat/control via MQTT (runs on root only)
 **********************************************************************
 **********************************************************************
 */


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    uint8_t dst_addr[MWIFI_ADDR_LEN]  = {0x0};
    mwifi_data_type_t data_type       = {0x0};

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/" CONFIG_MESH_ID "/+/control", 0);
            ESP_LOGI(TAG, "subscribed to \"/" CONFIG_MESH_ID "/+/control\", msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            MDF_LOGI("TOPIC=%.*s", event->topic_len, event->topic);
            MDF_LOGI("DATA=%.*s", event->data_len, event->data);
	    if( strncmp(event->topic, "/" CONFIG_MESH_ID "/",strlen("/" CONFIG_MESH_ID "/")) ) break;
	    if( strncmp(event->topic + strlen("/" CONFIG_MESH_ID "/123456123456"), "/control", strlen("/control")) ) break;
	    char* dev_addr=event->topic+strlen("/" CONFIG_MESH_ID "/");
	    for (int i = 0; i < MWIFI_ADDR_LEN; i++) { 
	        uint32_t ux;
	        sscanf(dev_addr+2*i,"%02x",&ux);
	        dst_addr[i]=ux;	    
	    }  
	    MDF_LOGI("mqtt dispatching to " MACSTR, MAC2STR(dst_addr)); 
            if(mwifi_write(dst_addr, &data_type, event->data, event->data_len, true) != MDF_OK)
	        MDF_LOGW("mqtt client failed to write to mesh"); 
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "MQTT_Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_stop(void) {
    if(g_mqtt_client == NULL) {
        MDF_LOGW("mqtt_stop: MQTT client is not running");
        return;
    }	
    g_mqtt_stop=1;
    esp_mqtt_client_stop(g_mqtt_client);
    esp_mqtt_client_destroy(g_mqtt_client);
    g_mqtt_client=NULL;
}  

static void mqtt_start(void)
{
    MDF_LOGI("Root starting mqtt client");

    // bail out if mqtt client task is running
    if(g_mqtt_client != NULL) {
        MDF_LOGW("mqtt_start: MQTT client is running");
        return;
    }	

    // configure
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_MQTT_BROKER_URL,
        .event_handle = mqtt_event_handler,
    };
    g_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    // start mqtt task
    if(esp_mqtt_client_start(g_mqtt_client)!=ESP_OK) {
        MDF_LOGW("Root failed to connect to mqtt broker");
        mqtt_stop();
	return;
    }	

    // report success
    MDF_LOGI("Root connected to mqtt broker");
}







/*
 **********************************************************************
 **********************************************************************
 * Node read and write
 **********************************************************************
 **********************************************************************
 */

// read and dispatch commands, typically received from root
static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN+1);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0};
    cJSON *json_root                  = NULL;
    cJSON *json_cmd                   = NULL;
    cJSON *json_t1                    = NULL;
    cJSON *json_t2                    = NULL;
    cJSON *json_t3                    = NULL;
    cJSON *json_version               = NULL;
    cJSON *json_board                 = NULL;
    cJSON *json_avrstate              = NULL;
    cJSON *json_avrimgcnt             = NULL;
    cJSON *json_avrdata               = NULL;
    cJSON *json_avraddr               = NULL;
    cJSON *json_avrcrc                = NULL;
    cJSON *json_avrpar                = NULL;
    cJSON *json_avrval                = NULL;
    char* command;

    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    //mesh_assoc_t mesh_assoc         = {0x0}; // 107 -- not functional, see below "esp_wifi_vnd_mesh_get"
    wifi_ap_record_t mesh_ap        = {0x0}; 
    mesh_addr_t parent_bssid        = {0};
    char *rdata   = NULL;
    size_t rsize   = 0;
    mwifi_data_type_t rdata_type     = {0};
      
    MDF_LOGI("Node read task is running");

    while (mwifi_is_connected()) {
        esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> node read failed (mwifi_read)", mdf_err_to_name(ret));
        MDF_LOGI("Node read: received data from " MACSTR ", size: %d", MAC2STR(src_addr), size);

	// dispatch: firmware upgrade data
        if (data_type.upgrade) { 
            ret = mupgrade_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_handle", mdf_err_to_name(ret));
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> root read failed (mupgrade_root_handle)", mdf_err_to_name(ret));
	    continue;
        }

	// strip "\r\n" .. purely cosmetic
	if( (data[size-1] == '\n') || (data[size-1] == '\r') ) {
	    data[size-1]=0;
  	    if( (data[size-2] == '\n') || (data[size-2] == '\r') ) 
	        data[size-2]=0;
	}
	data[size]=0;

	// dispatch: everything else is a JASON string
        MDF_LOGI("Node read: received data from " MACSTR ", size: %d, data: %.60s",
		 MAC2STR(src_addr), strlen(data), data);

        // parse and get command
        json_root = cJSON_Parse(data);
        MDF_ERROR_CONTINUE(!json_root, "node read parse error (cJSON_Parse)");
        json_cmd = cJSON_GetObjectItem(json_root, "cmd");
        MDF_ERROR_GOTO(!json_cmd, FREE_MEM, "node read parse error (missing \"cmd\")");
        command=json_cmd->valuestring;
        MDF_LOGI("Node read: command %s",command);

	// dispatch commands: restart this node
        if (!strcmp(command, "restart")) {
            MDF_LOGW("The device will restart after 10 seconds");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            MDF_LOGW("The device will restart now");
            esp_restart();
        } else {
	// dispatch commands: report mesh status
        if (!strcmp(command, "status")) {
	  //esp_wifi_vnd_mesh_get(&mesh_assoc); // 107 -- not functional (used for mesh_assoc.rsssi)
          esp_wifi_sta_get_ap_info(&mesh_ap);   // 107 -- functional alternative for rssi
	  esp_mesh_get_parent_bssid(&parent_bssid);
	  rsize = asprintf(&rdata,
	      "{\"src\":\"" MACSTR "\",\"mtype\":\"status\",\"parent\":\"" MACSTR "\",\"rssi\":%d,\"layer\":%d,\"nodes\":%d,\"plat\":%d}\r\n",
	     MAC2STR(sta_mac), MAC2STR(parent_bssid.addr), mesh_ap.rssi, esp_mesh_get_layer(),esp_mesh_get_total_node_num(),g_systime_roundtrip);
            ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { // tm: is there a macro fo rthis construct?
              MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	// dispatch commands: report system info
	} else {
        if (!strcmp(command, "system")) {
            esp_mesh_get_parent_bssid(&parent_bssid);
            rsize = asprintf(&rdata,
	        "{\"src\":\"" MACSTR "\",\"mtype\":\"system\",\"time\":%d,\"version\":\"%s\",\"board\":\"%s\",\"avrver\":%d}\r\n",
			     MAC2STR(sta_mac), systime(), DEMESH_VERSION, DEMESH_BOARD, g_tstate_version);
            ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { // tm: is there a macro fo rthis construct?
              MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	} else {
	// dispatch commands: time stamp roundtrip to synchronise systime
	if((!strcmp(command, "tsync")) || (!strcmp(command, "stsync"))) {
	    json_t1 = cJSON_GetObjectItem(json_root, "t1");
            json_t2 = cJSON_GetObjectItem(json_root, "t2");
            json_t3 = cJSON_GetObjectItem(json_root, "t3");
	    if(!json_t1) {
	        // no t1: this is a trigger for us to initiate the round trip by adding the t1 record
	        MDF_ERROR_GOTO(json_t2, FREE_MEM, "node read parse error (spourious parameter)");
	        MDF_ERROR_GOTO(json_t3, FREE_MEM, "node read parse error (spourious parameter)");
		if(esp_mesh_is_root()) {
		    MDF_LOGD("root will not sync systime with parent");
		    command_avrsettime();
		    goto FREE_MEM;
		}    
 	        json_t1=cJSON_CreateNumber(systime());
	        cJSON_AddItemToObject(json_root,"t1",json_t1);
	        rdata = cJSON_PrintUnformatted(json_root);
                esp_mesh_get_parent_bssid(&parent_bssid);
                ret = mwifi_write(parent_bssid.addr, &rdata_type, rdata, strlen(rdata), true); 
                if(ret != MDF_OK) { // tm: is there a macro fo rthis construct?
                    MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
		}
	    } else { 
            if(!json_t2) {
  	        // t1 but no t2: we are the parent of the initiator an add the t2 record
                MDF_ERROR_GOTO(!cJSON_IsNumber(json_t1), FREE_MEM, "node read parse error (t1 num)");
	        MDF_ERROR_GOTO(json_t3, FREE_MEM, "node read parse error (spourious parameter)");
	        json_t2=cJSON_CreateNumber(systime());
	        cJSON_AddItemToObject(json_root,"t2",json_t2);
	        rdata = cJSON_PrintUnformatted(json_root);
                ret = mwifi_write(src_addr, &rdata_type, rdata, strlen(rdata), true); 
                if(ret != MDF_OK) { // tm: is there a macro fo rthis construct?
                    MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
		}
	    } else {
            if(!json_t3) {
	        // t1 and t2 but no t3: we initiated the request and virtually fill in t3 for systime_adjust
                MDF_ERROR_GOTO(!cJSON_IsNumber(json_t1), FREE_MEM, "node read parse error (t1 num)");
                MDF_ERROR_GOTO(!cJSON_IsNumber(json_t2), FREE_MEM, "node read parse error (t2 num)");
	        TickType_t t3=systime();
	        if(systime_adjust(json_t1->valueint,json_t2->valueint,t3))
 	            command_avrsettime();
		// reply to root only if not self generated
		if(!strcmp(command, "tsync")) {
                    rsize = asprintf(&rdata,
	               "{\"src\":\"" MACSTR "\",\"mtype\":\"tsync\",\"tsync1\":%d,\"tsync2\":%d,\"tsync3\":%d}\r\n",
			   MAC2STR(sta_mac), json_t1->valueint,json_t2->valueint,t3);
                    ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
                    if(ret != MDF_OK) { // tm: is there a macro fo rthis construct?
                        MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
		    }
		}
	    }}}	        
	} else {
	// dispatch commands: initiate ESP32 OTA firmware upgrade (only root can do this)
        if (!strcmp(command, "upgrade")) {
  	    MDF_ERROR_GOTO(!esp_mesh_is_root(), FREE_MEM, "node read refuses upgrade command (not root)");
  	    MDF_ERROR_GOTO(g_upgrade_stage>0, FREE_MEM, "node read refuses upgrade command (upgrade pending)");
	    json_version = cJSON_GetObjectItem(json_root, "version");
            json_board = cJSON_GetObjectItem(json_root, "board");
	    MDF_ERROR_GOTO((!json_version) || (!json_board), FREE_MEM, "node read parse error (missing parameter)");
	    MDF_ERROR_GOTO((!cJSON_IsString(json_version)) || (!cJSON_IsString(json_board)),
	         FREE_MEM, "node read parse error (ill typed parameter)");
	    g_upgrade_stage=1;
	    g_upgrade_version=MDF_MALLOC(strlen(json_version->valuestring));
	    strcpy(g_upgrade_version,json_version->valuestring);
	    g_upgrade_board=MDF_MALLOC(strlen(json_board->valuestring));
	    strcpy(g_upgrade_board,json_board->valuestring);
	    xTaskCreate(upgrade_task, "upgrade_task", 4 * 1024,
                 NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	} else {
	// dispatch commands: set the avrota state for target AVR OTA firmware update   
        if (!strcmp(command, "avrota")) {
	    json_avrstate = cJSON_GetObjectItem(json_root, "state");
	    MDF_ERROR_GOTO(!json_avrstate, FREE_MEM, "node read parse error (missing parameter)");
	    MDF_ERROR_GOTO(!cJSON_IsString(json_avrstate), FREE_MEM, "node read parse error (ill typed parameter)");
	    // set to receive mode, aka accept firmware chunks from now on
	    if(!strcmp(json_avrstate->valuestring, "recimg")) {
	        ret=command_avrota_receive();
                if(ret == MDF_OK)
                    rsize = asprintf(&rdata,
		      "{\"src\":\"" MACSTR "\",\"mtype\":\"avrota\",\"state\":\"recimg\"}\r\n",MAC2STR(sta_mac));
		else
                    rsize = asprintf(&rdata,
		      "{\"src\":\"" MACSTR "\",\"mtype\":\"avrota\",\"state\":\"running\"}\r\n",MAC2STR(sta_mac));
   	    } else {
	    // trigger the flash process, aka flash all accumulated firmware chunks
	    if(!strcmp(json_avrstate->valuestring, "flash")) {
                json_avrimgcnt = cJSON_GetObjectItem(json_root, "avrimgcnt");
  	        MDF_ERROR_GOTO(!json_avrimgcnt, FREE_MEM, "node read parse error (missing parameter)");
	        MDF_ERROR_GOTO(!cJSON_IsNumber(json_avrimgcnt), FREE_MEM, "node read parse error (ill typed parameter)");
		ret=command_avrota_flash(json_avrimgcnt->valueint);
	        if(ret == MDF_OK)
                    rsize = asprintf(&rdata,
		      "{\"src\":\"" MACSTR "\",\"mtype\":\"avrota\",\"state\":\"running\"}\r\n",MAC2STR(sta_mac));
		else
                    rsize = asprintf(&rdata,
		      "{\"src\":\"" MACSTR "\",\"mtype\":\"avrota\",\"state\":\"halted\"}\r\n",MAC2STR(sta_mac));
	    } else {
	    // default: cancel any ongoing updates and reset to normal mode of operation
	        command_avrota_reset();
        	rsize = asprintf(&rdata,
		         "{\"src\":\"" MACSTR "\",\"mtype\":\"avrota\",\"state\":\"running\"}\r\n",MAC2STR(sta_mac));
	    }}			   
	    ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { 
              MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	} else {
	// dispatch command: receive a chunk of firmware for the target AVR    
        if (!strcmp(command, "avrimg")) {
	    json_avraddr = cJSON_GetObjectItem(json_root, "avraddr");
            json_avrdata = cJSON_GetObjectItem(json_root, "avrdata");
            json_avrcrc  = cJSON_GetObjectItem(json_root, "avrcrc");
	    MDF_ERROR_GOTO((!json_avraddr) || (!json_avrdata) || (!json_avrcrc),
		 FREE_MEM, "node read parse error (missing parameter)");
	    MDF_ERROR_GOTO((!cJSON_IsString(json_avrdata)) || (!cJSON_IsNumber(json_avraddr)) || (!cJSON_IsNumber(json_avrcrc)),
	         FREE_MEM, "node read parse error (ill typed parameter)");
	    ret = command_avrimage(json_avraddr->valueint, json_avrdata->valuestring, json_avrcrc->valueint);
	    if(ret == MDF_OK)
              rsize = asprintf(&rdata,
	        "{\"src\":\"" MACSTR "\",\"mtype\":\"avrimg\",\"avraddr\":%d,\"avrcrc\":\"ok\"}\r\n",MAC2STR(sta_mac), json_avraddr->valueint);
	    else
              rsize = asprintf(&rdata,
	        "{\"src\":\"" MACSTR "\",\"mtype\":\"avrimage\",\"avraddr\":%d,\"avrcrc\":\"error\"}\r\n",MAC2STR(sta_mac), json_avraddr->valueint);  
	    ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { 
              MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	} else {
	// dispatch command: set a parameter of the target AVR    
	if (!strcmp(command, "avrsetpar")) {
	    json_avrpar = cJSON_GetObjectItem(json_root, "avrpar");
            json_avrval = cJSON_GetObjectItem(json_root, "avrval");
	    MDF_ERROR_GOTO((!json_avrpar) || (!json_avrval),
		 FREE_MEM, "node read parse error (missing parameter)");
	    MDF_ERROR_GOTO((!cJSON_IsString(json_avrpar)) || (!cJSON_IsNumber(json_avrval)),
	         FREE_MEM, "node read parse error (ill typed parameter)");
	    ret = command_avrsetpar(json_avrpar->valuestring, json_avrval->valueint);
	    if(ret == MDF_OK)
                rsize = asprintf(&rdata,"{\"src\":\"" MACSTR "\",\"mtype\":\"avrsetpar\",\"avrpar\":\"ok\"}\r\n",MAC2STR(sta_mac));
	    else
  	        rsize = asprintf(&rdata,"{\"src\":\"" MACSTR "\",\"mtype\":\"avrsetpar\",\"avrpar\":\"fail\"}\r\n",MAC2STR(sta_mac));
	    ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { 
                MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	} else {
	// dispatch command: get a parameter from the target AVR    
	if (!strcmp(command, "avrgetpar")) {
	    json_avrpar = cJSON_GetObjectItem(json_root, "avrpar");
	    MDF_ERROR_GOTO(!json_avrpar,
		 FREE_MEM, "node read parse error (missing parameter)");
	    MDF_ERROR_GOTO(!cJSON_IsString(json_avrpar),
	         FREE_MEM, "node read parse error (ill typed parameter)");
	    int val;
	    ret = command_avrgetpar(json_avrpar->valuestring, &val);
	    if(ret == MDF_OK)
                rsize = asprintf(&rdata,"{\"src\":\"" MACSTR "\",\"mtype\":\"avrgetpar\",\"avrpar\":\"%s\",\"avrval\":%d}\r\n",
		             MAC2STR(sta_mac),json_avrpar->valuestring,val);
	    else
                rsize = asprintf(&rdata,"{\"src\":\"" MACSTR "\",\"mtype\":\"avrgetpar\",\"avrpar\":\"%s\",\"avrval\":\"fail\"}\r\n",
		             MAC2STR(sta_mac),json_avrpar->valuestring);
	    ret = mwifi_write(NULL, &rdata_type, rdata, rsize, true); 
            if(ret != MDF_OK) { 
              MDF_LOGD("<%s> mwifi_write", mdf_err_to_name(ret));
	    }  
	}}}}}}}}}

    FREE_MEM:

	// free cJSON memory and rdata buffer
        if(json_root) cJSON_Delete(json_root);
	json_root=NULL;
        if(rdata) MDF_FREE(rdata);
	rdata=NULL;
    } // while mifi_is_connected

    MDF_LOGW("Node read task is exit");
    vTaskDelete(NULL);
}


/*
// this is mesh-blink for testing aka ping
static void node_send_ping_task(void *arg)
{
    size_t size                     = 0;
    char *data                      = NULL;
    mdf_err_t ret                   = MDF_OK;
    mwifi_data_type_t data_type     = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Node ping task is running");

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    while(mwifi_is_connected()) {

        size = asprintf(&data, "{\"src\": \"" MACSTR "\",\"cmd\": \"ping\"}\r\n",
                        MAC2STR(sta_mac));

        MDF_LOGD("Node ping: send message to root, size: %d, data: %s", size, data);
        ret = mwifi_write(NULL, &data_type, data, size, true); 
        if(ret != MDF_OK) { // tm: is there a macro for this construct?
          MDF_LOGD("<%s> node ping mesh error (mwifi_write)", mdf_err_to_name(ret));
	}  
        MDF_FREE(data);
        vTaskDelay(10000 / portTICK_PERIOD_MS);

    }

    MDF_FREE(data);
    MDF_LOGW("Node ping task exit");

    vTaskDelete(NULL);
}
*/


// periodically request for synchronised system time
static void node_request_time_task(void *arg)
{
    size_t size                     = 0;
    char *data                      = NULL;
    mdf_err_t ret                   = MDF_OK;
    mwifi_data_type_t data_type     = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
 
   
    MDF_LOGW("Node request time has started");

    while(mwifi_is_connected()) {

        // resulary trigger round-trip calculus to sync systime();
        // this is received and handeled in node_read_task; the extra prefix "s"
        // in the command name "stsync" will allow us to distinguish self-generated
        // time scyns from externally ones
        esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
        size = asprintf(&data, "{\"cmd\":\"stsync\"}\r\n");
        MDF_LOGD("Node request time: send message to myself to trigger roundtrip: " MACSTR ", size: %d, data: %s",
	    MAC2STR(sta_mac),size,data);
        ret = mwifi_write(sta_mac, &data_type, data, size, true); 
        if(ret != MDF_OK) { // tm: is there a macro fo this construct?
            MDF_LOGD("<%s> node request time mesh error (mwifi_write)", mdf_err_to_name(ret));
        }  
        MDF_FREE(data);

	// wait for 10+/-1 minutes 
        vTaskDelay(600000 / portTICK_PERIOD_MS );
        vTaskDelay( esp_random() / ((0xffffffffU /  60000) / portTICK_PERIOD_MS ) ) ;

    }

    MDF_FREE(data);
    MDF_LOGW("Node request time task is exit");

    vTaskDelete(NULL);
}

/*
 **********************************************************************
 **********************************************************************
 * Periodic heartbeat sysinfo via mesh to root 
 * (both tcp socket and publich via mqtt)
 **********************************************************************
 **********************************************************************
 */

// access to the one and only heartbeat task per node
TaskHandle_t g_heartbeat_task = NULL;

// wake up heartbeat
static void heartbeat_trigger(void)
{
    if(g_heartbeat_task==NULL) return;
    xTaskNotifyGive(g_heartbeat_task);
}  

// send heartbeat every 5secs (or programatic on trigger "heartbeat_trigger")
static void heartbeat_task(void *qrg)
{

    size_t size                     = 0;
    char *data                      = NULL;
    mdf_err_t ret                   = MDF_OK;
    mwifi_data_type_t data_type     = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    //mesh_assoc_t mesh_assoc         = {0x0}; // 107 -- not functional, see below
    wifi_ap_record_t mesh_ap        = {0x0};   // 107 -- functional alternative for rssi
   
    // hello
    MDF_LOGW("Heartbeat task starting");
    g_heartbeat_task=xTaskGetCurrentTaskHandle();

    // insist in mesh 
    while(mwifi_is_connected()) {

        // get mesh data
        esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
        esp_wifi_sta_get_ap_info(&mesh_ap);    // 107 -- functional alternative for rssi
        // esp_wifi_vnd_mesh_get(&mesh_assoc); // 107 -- not functional

	// get target uC state
	command_avrgetstate();
	
        // compose message
        size = asprintf(&data,
           "{\"src\":\"" MACSTR "\",\"mtype\":\"heartbeat\",\"rssi\":%d,\"ccss\":%d,\"maxcur\":%d,\"phases\":%d,\"cur1\":%d,\"cur2\":%d,\"cur3\":%d}\r\n",
	   MAC2STR(sta_mac), mesh_ap.rssi, g_tstate_ccss, g_tstate_maxcur, g_tstate_phases, g_tstate_cur1, g_tstate_cur2, g_tstate_cur3);

        // send message
        ret = mwifi_write(NULL, &data_type, data, size, true); 
        if(ret != MDF_OK) { // tm: is there a macro fo this construct?
            MDF_LOGD("<%s> node heartbeat mesh error (mwifi_write)", mdf_err_to_name(ret));
        }  
        MDF_FREE(data);

	// delay
	ulTaskNotifyTake(true, g_heartbeat_period / portTICK_PERIOD_MS );
    }

    // goodbye
    MDF_LOGW("Heartbeat task terminating");
    g_heartbeat_task=NULL;
    vTaskDelete(NULL);
}

/*
 **********************************************************************
 **********************************************************************
 * Periodic sysinfo on stdout for basic debugging
 **********************************************************************
 **********************************************************************
 */

static void print_system_info_timercb(void *timer)
{
    uint8_t primary                 = 0;
    wifi_second_chan_t second       = 0;
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
 
    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_get_channel(&primary, &second);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, version v%s, systime %d, channel: %d, layer: %d, node: " MACSTR
      ", parent: " MACSTR ", node cnt: %d, free heap: %u",
       DEMESH_VERSION, systime(), primary,
       esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
	 esp_mesh_get_total_node_num(), esp_get_free_heap_size());

#ifdef MEMORY_DEBUG
    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
    mdf_mem_print_task();
#endif /**< MEMORY_DEBUG */

}

/*
 **********************************************************************
 **********************************************************************
 * Mesh maintenance 
 **********************************************************************
 **********************************************************************
 */


// esp-mdf main event loop callback 
static mdf_err_t mesh_event_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event loop: esp-mdf %d", event);

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("event callback: mwifi is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("event callback: parent is connected on station interface");
	    if (esp_mesh_is_root()) {
	      esp_netif_dhcpc_start(g_netif_sta); 
            }
            xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            xTaskCreate(node_request_time_task, "node_request_time_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            xTaskCreate(heartbeat_task, "heartbeat_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            if (esp_mesh_is_root()) {
	        xTaskCreate(root_read_task, "root_read_task", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            }
	    break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("event callback: parent is disconnected from station interface");
            break;

        case MDF_EVENT_MWIFI_ROUTING_TABLE_ADD:
        case MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE:
            MDF_LOGI("event callback: mesh configuration changed, number if nodes %d", esp_mesh_get_total_node_num());
            break;

        case MDF_EVENT_MWIFI_ROOT_GOT_IP: {
            MDF_LOGI("event callback: root obtains the IP address");
            xTaskCreate(upstream_read_task, "upstream_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	    mqtt_start();
            break;
        }

	case MDF_EVENT_MUPGRADE_STARTED: {
            mupgrade_status_t status = {0x0};
            mupgrade_get_status(&status);
            MDF_LOGI("event callback: received firmware upgrade proposal, name: %s, size: %d",status.name, status.total_size);
	    int match=true;
            if(strncmp(status.name,CONFIG_FIRMWARE_BASENAME,strlen(CONFIG_FIRMWARE_BASENAME)))
 	        match=false;
	    if(!strstr(status.name,DEMESH_BOARD))
                match=false;
	    if(!match) {
	       MDF_LOGI("event callback: firmware upgrade rejected (prefix and/or board misatch)");	      
               mupgrade_stop();
               break;	       
	    }   
	    if(!strncmp(status.name+strlen(CONFIG_FIRMWARE_BASENAME)+2,DEMESH_VERSION " ",strlen(DEMESH_VERSION)+1)) {
	       MDF_LOGI("event callback: firmware upgrade rejected (already up-to date)");
               mupgrade_stop();
               break;
	    }   
	    MDF_LOGI("event callback: firmware upgrade accepted");
	    g_upgrade_stage=5;
            break;
        }

        case MDF_EVENT_MUPGRADE_STATUS:
            MDF_LOGI("event callback: upgrade progress: %d%%", (int)ctx);
            break;

        case MDF_EVENT_MUPGRADE_STOPED:
  	    MDF_LOGI("event callback: upgrade stoped");
	    g_upgrade_stage=0;
	    break;
	    
        case MDF_EVENT_MUPGRADE_FINISH:
  	    MDF_LOGI("event callback: upgrade finished");
	    g_upgrade_stage=0;
	    break;

        default:
            break;
    }
    return MDF_OK;
}


// initialise mwifi
static mdf_err_t mesh_init()
{

    // have mdf event loop, effectively drives all our code
    MDF_ERROR_ASSERT(mdf_event_loop_init(mesh_event_cb));

    // start wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    MDF_ERROR_ASSERT(esp_netif_init()); 
    MDF_ERROR_ASSERT(esp_event_loop_create_default()); 
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&g_netif_sta, NULL)); 
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg)); 
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    // start mesh
    mwifi_init_config_t mcfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t mconfig     = {
        .router_ssid     = CONFIG_ROUTER_SSID,
        .router_password = CONFIG_ROUTER_PASSWORD,
        .mesh_id         = CONFIG_MESH_ID,
        .mesh_password   = CONFIG_MESH_PASSWORD,
    };
    MDF_ERROR_ASSERT(mwifi_init(&mcfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&mconfig));
    MDF_ERROR_ASSERT(mwifi_start());

    return MDF_OK;
}



/*
 **********************************************************************
 **********************************************************************
 * Target AVR Debugging
 * 
 * We set up a SoftAP for the host to connect --- cannot be used simultaneously
 * with the mesh. 
 *
 * We run a telnet server on port 23 and forward all trafic bidirectional to 
 * the AVR serial line. Here, we operate line based and expect "printable ASCII" 
 * only. Connect via telnet and terminate the connection by ^D or ^Q. 
 *
 * For AVR programming, we also run a TCP server on port 2323 which resets 
 * the AVR on connection to enter optiboot. Here, we forward all trafic 
 * completely transparently to the AVR serial line. Connect to this service
 * with e.g. "avrdude" using port "-P net:<IP address>:2323"
 *
 * (as of Dec 2020, this has not been tested with ESP_MDF v1.0)
 *
 *
 **********************************************************************
 **********************************************************************
 */

#define SOFTAP_CHANNEL 7

int g_tcpcon_telnet=0;    // 0: not running; 1: running; -1: ask to terminate
int g_tcpcon_optiboot=0;  // 0: not running; 1: running; -1: ask to terminate

// Pass through AVR serial as telnet server (cast arg to socket):
void debug_telnet_server_task(void *arg) {

    int sockfd=-1;
    int res;
    int pos;
    char data[MWIFI_PAYLOAD_LEN+1];
    size_t size                       = MWIFI_PAYLOAD_LEN;

    // get my socket from task argument
    sockfd = (int) arg;

    // only one such task allowed
    if(g_tcpcon_telnet!=0) {
       close(sockfd);
       return;
    }
    g_tcpcon_telnet=1;
        
    // up and running
    if(g_tcpcon_optiboot!=1) g_blinks=2;
    MDF_LOGI("Debug AVR telnet: server on socket %d",sockfd);

    // make nonblocking
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

    // loop comms
    while(g_tcpcon_telnet==1) {

        // optiboot has priority
        if(g_tcpcon_optiboot==1) {
	    vTaskDelay(250 / portTICK_RATE_MS);
	    continue;
        }

	// read from socket (if telnet is in line mode this will be one line)
	size=0;
        res = read(sockfd, data, MWIFI_PAYLOAD_LEN);
        if ( (res < 0) &&  (errno!=EAGAIN) ) {
  	    MDF_LOGW("<%s> (%d) read", strerror(errno),errno);
            break;
        };
	if(res>0) {
    	    size=res;	  
  	    data[size]=0;
            MDF_LOGD("Debug AVR telnet: received message, size: %d, data: %.20s", size, data);
	}    

	// figure end-of-transmission (ctrl-D) in ASCII mode (or alt. ctrl-Q)
 	if( (index(data,0x04)!=NULL) || (index(data,0x11)!=NULL) ) {
  	    MDF_LOGD("Debug AVR telnet: ASCII mode end-of-transmission (^D or ^Q)");
	    break;
        }
		
	// pass to serial
   	pos=0;
    	while(pos<size) {
            uart_wait_tx_done(UART_NUM_1, 1000 / portTICK_RATE_MS);
            res = uart_write_bytes(UART_NUM_1,data+pos,size-pos);
    	    if(res<0) break;
	    pos+=res;
        }
        if(pos<size) {
            MDF_LOGW("uart write: buffer overflow");
            break;
        }

	// read from serial
  	MDF_ERROR_ASSERT(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&size));
        if(size> MWIFI_PAYLOAD_LEN)
	    size=MWIFI_PAYLOAD_LEN;
        if(size>0)
            size = uart_read_bytes(UART_NUM_1, (unsigned char*) data, size, 0);    

	// pass to socket
        if(size>0) {
  	    data[size]=0;
            MDF_LOGD("Debug AVR telnet: sending message, size: %d, data: %.20s", size, data);
     	    pos=0;
    	    while(pos<size) {
                res = write(sockfd, data+pos, size-pos);
	        if(res<0) break;
	        pos+=res;
	    }
	    if(pos<size) {
                MDF_LOGW("<%s> write", strerror(errno));
		break;
            }
	}

        // give other tasks more chances during optiboot
	vTaskDelay(10 / portTICK_RATE_MS);
 
    }

    // disconected (cancel request, EOT, our socket error)
    if(sockfd>=0) close(sockfd);
    MDF_LOGI("Debug AVR telnet: server connection closed on %d",sockfd);
    if(g_tcpcon_optiboot!=1) g_blinks=5;
    g_tcpcon_telnet=0;
    vTaskDelete(NULL);
}


// Pass through AVR optiboot as TCP server (cast arg to socket):
void debug_optiboot_server_task(void *arg) {

    int sockfd=-1;
    int res;
    int pos;
    char data[MWIFI_PAYLOAD_LEN+1];
    size_t size                       = MWIFI_PAYLOAD_LEN;
    TickType_t rtime=0;
    TickType_t now=0;
    int mode=0;

    // get my socket from task argument
    sockfd = (int) arg;

    // only one such task allowed
    if(g_tcpcon_optiboot!=0) {
       close(sockfd);
       return;
    }
    g_tcpcon_optiboot=1;
        
    // up and running
    g_blinks=2;
    MDF_LOGI("Debug AVR optiboot: server on %d",sockfd);

    // make nonblocking
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

    // set timers
    rtime=xTaskGetTickCount();

    // halt avr
    avrreset(1);
    avrclearln();
    vTaskDelay(10 / portTICK_PERIOD_MS);

    // loop comms
    while(g_tcpcon_optiboot==1) {

	// read clock
	now=xTaskGetTickCount();

	// figure time-outs: no transmission since 2secs
	if((mode==1) && (now - rtime > 2000/portTICK_PERIOD_MS)) {
  	    MDF_LOGD("Debug AVR optiboot: protocol time-out");
	    break;
	}

	// figure time-outs: no transmission started since connect (1sec)
	if((mode==0) && (now - rtime > 1000/portTICK_PERIOD_MS)) {
  	    MDF_LOGD("Debug AVR optiboot: transmission time-out");
	    break;
	}
      
	// read from socket
	size=0;
        res = read(sockfd, data, MWIFI_PAYLOAD_LEN);
        if ( (res < 0) &&  (errno!=EAGAIN) ) {
  	    MDF_LOGW("<%s> (%d) read", strerror(errno),errno);
            break;
        }
	if(res>0) {
    	    size=res;	  
  	    data[size]=0;
	    MDF_LOGD("Debug AVR optiboot: received command, size: %d", res);
	    ESP_LOG_BUFFER_HEX("XMEGA OPTI CMD", data,size);
   	    rtime=xTaskGetTickCount();
	}
	   
	// sense transmission start
	if((mode==0) && (size>1)) {	  
  	    MDF_LOGD("Debug AVR optiboot: transmission started");
	    mode=1;
            avrreset(0);
            vTaskDelay(10 / portTICK_PERIOD_MS);
	    avrclearln();
	    g_blinks=20;
	    // swallow the first sync (some avrdude need this)
	    size=0;
	}	
	

	// pass to serial
   	if(size>0) {
            for(pos=0; pos<size;) {
                res = uart_write_bytes(UART_NUM_1,data+pos,size-pos);
                uart_wait_tx_done(UART_NUM_1, 1000 / portTICK_RATE_MS);
                if(res<0) break;
	        pos+=res;
            }
	    if(pos<size) {
                MDF_LOGW("uart write: buffer overflow");
	        break;
            }
	    rtime=xTaskGetTickCount();
	}    

	// read from serial
  	MDF_ERROR_ASSERT(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&size));
        if(size> MWIFI_PAYLOAD_LEN)
            size=MWIFI_PAYLOAD_LEN;
        if(size>0) 
            size = uart_read_bytes(UART_NUM_1, (unsigned char*) data, size, 0);
        if(size>0)
	    rtime=xTaskGetTickCount();
	  
	// pass to socket
        if((size>0) && (mode==1)) {
	    MDF_LOGD("Debug AVR optiboot: sending reply, size: %d", size);
	    ESP_LOG_BUFFER_HEX("XMEGA OPTI CMD", data,size);
     	    pos=0;
    	    while(pos<size) {
                res = write(sockfd, data+pos, size-pos);
	        if(res<0) break;
	        pos+=res;
	    }
	    if(pos<size) {
                MDF_LOGW("<%s> write", strerror(errno));
		break;
            }
	    rtime=xTaskGetTickCount();
	}

        // give other tasks more chances
	vTaskDelay(10 / portTICK_RATE_MS);
 
    }

    // disconnected (cancel request, time-out , our socket error)
    if(sockfd>=0) close(sockfd);
    MDF_LOGI("Debug AVR optiboot: server connection closed on %d",sockfd);
    g_tcpcon_optiboot=0;
    g_blinks = g_tcpcon_telnet==1 ? 2 : 5;
    // vTaskDelete(NULL); // we are currently not running this as a task  
}
    

// run TCP server to listen for telnet connections
void debug_telnet_listen_task(void *arg)
{
    struct sockaddr_in remaddr;
    int sockfd;
    
    MDF_LOGI("Debug TCP server listening for telnet connections");
    g_blinks=5;

    while (true) {
       	
        // create a tcp server 
        if (g_sockfd_telnet == -1) {

            MDF_LOGI("Debug: create a tcp server, ip: %s, port: %d",
		CONFIG_DEBUG_SERVER_IP, CONFIG_DEBUG_TELNET_PORT);

            // create socket
	    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	    if(sockfd<0) {
                MDF_LOGI("Debug: failed to create socket");
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		continue;
	    }

	    // make reusable
            int enable=1;
	    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
		      
	    // set myself as server address 
	    g_server_addr.sin_family = AF_INET;
            g_server_addr.sin_port = htons(CONFIG_DEBUG_TELNET_PORT);
            g_server_addr.sin_addr.s_addr = inet_addr(CONFIG_DEBUG_SERVER_IP);

	    // bind socket to listen
	    if (bind(sockfd, (struct sockaddr *) &g_server_addr, sizeof(g_server_addr)) < 0) {
                MDF_LOGI("Debug: failed to bind socket");
		close(sockfd);
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		continue;
            }

	    // success, we are now listening
	    g_sockfd_telnet=sockfd;
	}

        // wait for connection (handle only one)
        MDF_LOGI("Debug server waiting for telnet client to connect");
	listen(g_sockfd_telnet,5);
        size_t remlen = sizeof(remaddr);
        sockfd = accept(g_sockfd_telnet, (struct sockaddr *)&remaddr, &remlen);
	if (sockfd < 0) {
            MDF_LOGI("Debug: failed to accept incommming connection");
     	    close(g_sockfd_telnet);
	    g_sockfd_telnet=-1;
	    vTaskDelay(2000 / portTICK_PERIOD_MS);
	    continue;
        }

	// report connection
	MDF_LOGI("Debug: telnet client connected, ip: %s on socket %d",
	    inet_ntoa( remaddr.sin_addr ), sockfd);

	// cancel other telnet task, assume it to be stale (should use mutex and wait for task ended)
	if(g_tcpcon_telnet!=0) {
	    MDF_LOGI("Debug: killing (stale?) telnet connection");
	    g_tcpcon_telnet=-1;
 	    while(g_tcpcon_telnet!=0) vTaskDelay(250 / portTICK_PERIOD_MS);
	}
	  
	// start task for this connection
        xTaskCreate(debug_telnet_server_task, "debug_server_telnet_task", 4 * 1024,
		    (void*) sockfd, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
 
    }

    // error exit
    if(g_sockfd_telnet>=0) close(g_sockfd_telnet);
    MDF_LOGI("Debug TCP server died");
    vTaskDelete(NULL);

}

// run TCP server to listen for optiboot connections
void debug_optiboot_listen_task(void *arg)
{
    struct sockaddr_in remaddr;
    int sockfd;
    
    MDF_LOGI("Debug TCP server listening for optiboot connections");
    g_blinks=5;

    while (1) {
       	
        // create a tcp server 
        if (g_sockfd_optiboot == -1) {

            MDF_LOGI("Debug: create a tcp server, ip: %s, port: %d",
		CONFIG_DEBUG_SERVER_IP, CONFIG_DEBUG_OPTIBOOT_PORT);

            // create socket
	    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	    if(sockfd<0) {
                MDF_LOGI("Debug: failed to create socket");
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		continue;
	    }

	    // make reusable
            int enable=1;
	    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
		      
	    // set myself as server address 
	    g_server_addr.sin_family = AF_INET;
            g_server_addr.sin_port = htons(CONFIG_DEBUG_OPTIBOOT_PORT);
            g_server_addr.sin_addr.s_addr = inet_addr(CONFIG_DEBUG_SERVER_IP);

	    // bind socket to listen
	    if (bind(sockfd, (struct sockaddr *) &g_server_addr, sizeof(g_server_addr)) < 0) {
                MDF_LOGI("Debug: failed to bind socket");
		close(sockfd);
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		continue;
            }

	    // success, we are now listening
	    g_sockfd_optiboot=sockfd;
	}

        // wait for connection (handle only one)
        MDF_LOGI("Debug server waiting for optiboot client to connect");
	listen(g_sockfd_optiboot,2);
        size_t remlen = sizeof(remaddr);
        sockfd = accept(g_sockfd_optiboot, (struct sockaddr *)&remaddr, &remlen);
	if (sockfd < 0) {
            MDF_LOGI("Debug: failed to accept incommming connection");
     	    close(g_sockfd_optiboot);
	    g_sockfd_optiboot=-1;
	    vTaskDelay(2000 / portTICK_PERIOD_MS);
	    continue;
        }

	// report connection
	MDF_LOGI("Debug: optiboot client connected, ip: %s on socket %d",
	    inet_ntoa( remaddr.sin_addr ), sockfd);

	// run optiboot connection in this task
        debug_optiboot_server_task((void*) sockfd);
 
    }

    // error exit
    if(g_sockfd_optiboot>=0) close(g_sockfd_optiboot);
    MDF_LOGI("Debug TCP server died");
    vTaskDelete(NULL);

}


// SoftAP event handler
static void softap_event_cb(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{

    wifi_sta_list_t stations;
    wifi_event_ap_staconnected_t* conevent;
    wifi_event_ap_stadisconnected_t* disevent;

    switch(event_id) {
    case WIFI_EVENT_AP_STACONNECTED:
        conevent = (wifi_event_ap_staconnected_t*) event_data;
        MDF_LOGI("softap event: station:"MACSTR" join, AID=%d",
                 MAC2STR(conevent->mac), conevent->aid);
	MDF_ERROR_ASSERT(esp_wifi_ap_get_sta_list(&stations));
	if(g_station_cnt==0) {
	}
        break;
    case WIFI_EVENT_AP_STADISCONNECTED:
        disevent = (wifi_event_ap_stadisconnected_t*) event_data;
        MDF_LOGI("softap event: station:"MACSTR" leave, AID=%d",
                 MAC2STR(disevent->mac), disevent->aid);
	MDF_ERROR_ASSERT(esp_wifi_ap_get_sta_list(&stations));
	g_station_cnt=stations.num;
	if(g_station_cnt==0) g_blinks=10;
        break;
    default:
        MDF_LOGI("softap event: unhandled");
        break;
    }
}

// set up and start SoftAP
static mdf_err_t softap_init(void)
{
    g_blinks=10;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &softap_event_cb, NULL, NULL));
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_SOFTAP_SSID,
            .ssid_len = strlen(CONFIG_SOFTAP_SSID),
            .channel = SOFTAP_CHANNEL,
            .password = CONFIG_SOFTAP_PASSWORD,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(CONFIG_SOFTAP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_AP));
    MDF_ERROR_ASSERT(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    MDF_ERROR_ASSERT(esp_wifi_start());

    MDF_LOGI("wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_SOFTAP_SSID, CONFIG_SOFTAP_PASSWORD, SOFTAP_CHANNEL);

    // start my servers
    xTaskCreate(debug_telnet_listen_task, "debug_telnet_listen_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    xTaskCreate(debug_optiboot_listen_task, "debug_optiboot_listen_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    return MDF_OK;
}






/*
 **********************************************************************
 **********************************************************************
 * Application entry point aka "main"
 **********************************************************************
 **********************************************************************
 */

void app_main()
{
  
    // log levels
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mupgrade_node", ESP_LOG_DEBUG);
    esp_log_level_set("mupgrade_root", ESP_LOG_DEBUG);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    // board devices (LEDs, TFT,  etc, except for the blink led)
    init_devices();

    // figure whether we go to debug mode (1sec boot delay)
#ifdef DEBUG_GPIO    
    g_debug_target = ( gpio_get_level(DEBUG_GPIO) == DEBUG_ON );
#endif    
   
    // run blink
    xTaskCreate(blink_task, "blink_task", 4 * 1024,
        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    // initialise avrflash partition
#ifdef AVR_PRESENT    
    g_avrimg_part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY,"avrimg");
    MDF_ERROR_ASSERT(g_avrimg_part!=NULL ? MDF_OK : MDF_FAIL);
#endif    


    // initialise UART1 for AVR (using 256 bytes for RX and TX buffers, resp.)
#ifdef AVR_PRESENT    
    const uart_config_t uart_config = {
        .baud_rate    =  AVR_BAUDRATE,
        .data_bits    =  UART_DATA_8_BITS,
        .parity       =  UART_PARITY_DISABLE,
        .stop_bits    =  UART_STOP_BITS_1,
        .flow_ctrl    =  UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, AVR_TXD_GPIO, AVR_RXD_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 256, 256, 0, NULL, 0));

    // test uart e.g. with hardware echo
    /*
    while(1) {
      char reply[256];
      int cnt;
      cnt= uart_write_bytes(UART_NUM_1, "HelloWorldEsp\r\n", 15);
      uart_wait_tx_done(UART_NUM_1, 1000 / portTICK_RATE_MS);
      MDF_LOGD("tx: #%d bytes", cnt);
      cnt = uart_read_bytes(UART_NUM_1, (unsigned char*) reply, 256, 1000 / portTICK_RATE_MS);
      reply[cnt]=0;
      MDF_LOGD("rx: %s (#%d)", reply,cnt);
    } 
    */

    // have a mutex for AVR UART communications
    g_avruart_mutex = xSemaphoreCreateMutex();

#endif    

    // both mesh and aoftap need nvs
    mdf_err_t ret=nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    MDF_ERROR_ASSERT(ret);

    // start network
    if(!g_debug_target) {
        // start mesh, incl. event loop --- this drives all of our main functionallity
        MDF_ERROR_ASSERT(mesh_init());
    } else {
        // start softap, incl. debug event loop --- this drives all of our target uC debug functionallity
        MDF_ERROR_ASSERT(softap_init());
    }

    
#ifndef AVR_PRESENT
    // start target uC simulation 
    TimerHandle_t stimer = xTimerCreate("simulate_target", 1000 / portTICK_PERIOD_MS,
                                       true, NULL, simulate_target_timercb);
    xTimerStart(stimer, 0);
#endif    

    // start periodic sysinfo
    if(!g_debug_target) {
        TimerHandle_t ptimer = xTimerCreate("print_system_info", 10000 / portTICK_PERIOD_MS,
                                       true, NULL, print_system_info_timercb);
        xTimerStart(ptimer, 0);
    }	



}
