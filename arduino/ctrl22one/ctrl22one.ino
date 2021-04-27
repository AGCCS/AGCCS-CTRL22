/*
ESP32 firmware for the control of one stand-alone AGCCS-CTRL22 charging 
station.

Disclaimer: we use this software for testing and further development of
the target AVR firmwarem i.e., we charge various cars and learn how 
they differ in their behaviour regarding wake/sleep etc; in particular,
this software in its current stage is presumably not suitable for
production use.

Copyright (c) Thomas Moor, 2021.


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// AGCCS version (one digit major, one digit minor)
#define VERSION 11
 
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <EEPROM.h>
#include <HardwareSerial.h>


// select board
//#define M5STICK
#define AGCCS12


// ctrl22one configuration
bool   g_cfg_brkxen  = true;
String g_cfg_brkxurl = "mqtt://192.168.2.108:1884";
String g_cfg_brkxusr = "???";
String g_cfg_brkxpwd = "???";
String g_cfg_brkxtpc = "CTRL22ONE";
String g_cfg_stassid = "???";
String g_cfg_stapwd  = "???";
String g_cfg_sappwd  = "ctrl22one";
String g_cfg_devname = "???";
int g_cfg_opticnt = 0;


// ctrl22one globals 
WiFiClient       g_network;
String           g_devname;
bool             g_stamode = false;

// all-purpose temp JSON buffer
#define JSONBUF 256
StaticJsonDocument<JSONBUF> g_jdocbuf;
char g_jstrbuf[JSONBUF];


// local copy of target AVR state
static int g_tstate_version=-1;   // target uC firmware version                                                                                
static int g_tstate_ccss=-1;      // state in CCS charging scheme (0<>idle ... 30<> charging, >=50 error)                                      
static int g_tstate_sphases=-1;   // phases allocated by remote server                                                                         
static int g_tstate_aphases=-1;   // actual phases enabled by SSRs (matches sphases in simulation)                                             
static int g_tstate_imaxcur=-1;   // max current by installation
static int g_tstate_smaxcur=-1;   // max current allocated by remote server (per phase, unit 100mA, aka "10" reads "1A")                       
static int g_tstate_sonoff=1;     // on/off by remote server                       
static int g_tstate_cmaxcur=-1;   // max current allowed by charging cable (per phase, unit 100mA)                                             
static int g_tstate_amaxcur=-1;   // actual max current as indicated by CP (unit 100mA, matches min(smaxcur,cmaxcur) in simulation)            
static int g_tstate_cur1=-1;      // current drawn on phase 1 (unit 100mA)                                                                     
static int g_tstate_cur2=-1;      // current drawn on phase 1 (unit 100mA)                                                                     
static int g_tstate_cur3=-1;      // current drawn on phase 1 (unit 100mA)      

// global target AVR control variables
static int g_tcontrol_imaxpow=-1; // set to nonnegative max installed power (unit 0.1kW)
static int g_tcontrol_sphases=-1; // set to nonnegative for available phases
static int g_tcontrol_smaxpow=-1; // set to nonnegative desired power (unit 0.1kW)
static int g_tcontrol_sonoff=-1;  // set to nonnegative on/off i.e. 1/0 
static String g_tcontrol_vrbcmd=""; // set to nonemty for a verbatim command 

// forward: development/testing log via websocket
void wsck_log(const String&);


/* 
**************************************************************************  
**************************************************************************  

Minimal hardware abstraction layer
- we conveniently use an M5StickC for development/testing
- or our AGCCS rev 1.2 board

**************************************************************************  
**************************************************************************  
*/

#ifdef M5STICK
// simple setup M5StickC 
// - LED only, dont use TFT at this stage
// - could use any ESP32 dev board instead

static const int GPIOLED=10;

// LED on GPIO10, active low
void led_on(bool on=true) { 
  digitalWrite(GPIOLED,on ? LOW : HIGH); 
}  
void led_off() { 
  digitalWrite(GPIOLED,HIGH); 
}
void dev_init() {
  pinMode(GPIOLED, OUTPUT);
  digitalWrite(GPIOLED,LOW);  
}

// simulate target AVR
#define AVRSIMULATE
#undef ARVSERIAL
#undef ARVUPDATE

#endif



#ifdef AGCCS12
// AGCCS CTRL22 board, rev 12
// - use LED on programming adaptor J5
// - use serial/rst conection to AVR

static const int GPIOLED=0;
static const int GPIORST=12;
static const int GPIORXD=16;
static const int GPIOTXD=17;

// LED on GPIO0, active low
void led_on(bool on=true) { 
  digitalWrite(GPIOLED,on ? LOW : HIGH); 
}  
void led_off() { 
  digitalWrite(GPIOLED,HIGH); 
}

// initialize incl serial monitor
void dev_init() { 
  // LED driver on GPIO0
  pinMode(GPIOLED, OUTPUT);
  digitalWrite(GPIOLED,LOW);  
  // enable reset line
  pinMode(GPIORST, OUTPUT);
  digitalWrite(GPIORST,HIGH);
  // initialise serial (RXD GPIO16, TXD GPIO17)
  Serial2.begin(115200,SERIAL_8N1,GPIORXD,GPIOTXD);
  pinMode(GPIORXD, INPUT_PULLUP);
  pinMode(GPIOTXD, OUTPUT);
}

// do talk to attached traget AVR
#define AVRSERIAL
#define AVRUPDATE
#undef AVRSIMULATE

#endif


/* 
**************************************************************************  
**************************************************************************  

Organize systime
- call onne per main loop
- have ms ticks with 3600*1000 rollover
- have sec ticks with 3600 rollover
- have 1 second trigger
  
**************************************************************************  
**************************************************************************  
*/ 

unsigned long    g_systime=0;
bool             g_trig1sec=false;

void systime_lcb(void) {

  // organize systime
  static unsigned long offsetms=0; 
  static unsigned long schedsec=0; 
  static unsigned long nextsec=0;  
  g_systime=millis() +offsetms;
  if(g_systime>3600L*1000L) {
    offsetms-=3600L*1000L;
    g_systime-=3600L*1000L;
    schedsec-=3600L*1000L;
  }
}

// utility: simple timer/delay
// - max delay half of rollover, i.e., 3600x1000/2
class Timer {
public:
  Timer(void) : schd(-1), dly(0), rld(false) {}
  Timer(int d, bool r=false) : dly(d), rld(r) {start();}  
  void set(int d, bool r=false) {
    dly=d;
    rld=r;
    start();
  } 
  void clear(void) {
    schd=-1;
    if(rld) start();
  } 
  bool expired(void) {
    bool res=false;
    if(schd==-1) return res;
    if((g_systime >= schd) && ( (g_systime - schd) < 3600L*1000L/2 )) res=true;
    if((schd > g_systime) && ( (schd - g_systime) > 3600L*1000L/2 )) res=true;
    if(!res) return res;
    if(rld) 
      schd= (schd+ dly) % (3600L*1000L);
    else
      schd=-1;
    return true;
  }
  bool running(void) {
    expired();
    return schd!=-1;
  }
protected:
  void start(void) {
    schd=(g_systime + dly) % (3600L*1000L);
  }  
  int schd;
  int dly;     
  bool rld;
};

// utility periodics 
// - period should be a divisor of rollover 3600x1000
class Periodic : protected Timer {
public:  
  Periodic(int prd, int off=0) : Timer() {
    dly=prd;
    rld=true;
    schd= ( ((g_systime / dly) *dly) + dly + off )   % (3600L*1000L);
  }
  bool expired(void) {
    return Timer::expired();
  }    
};



/* 
**************************************************************************  
**************************************************************************  

Utilities
- conver numeric version to string
  
**************************************************************************  
**************************************************************************  
*/ 

String verstr(int ver) {
  if(ver<=0) return String("--");
  return String(ver/10)+"."+String(ver%10);
}

/* 
**************************************************************************  
**************************************************************************  

Maintain configuration in EEPROM
- pragmatically use JSON as encoding since we use this later anyway
  and do not want to bother about individial string length and such.
  
**************************************************************************  
**************************************************************************  
*/ 

// conversion from JSON
bool cfg_fromjson(void) {
  JsonObject jobj=g_jdocbuf.as<JsonObject>();
  String pwd;
  bool ret=false;
  for(JsonPair kv : jobj) {
    if(kv.key()=="brkxen")
    if(kv.value().is<int>()) {
      g_cfg_brkxen=kv.value().as<int>();
      ret=true;
    }
    if(kv.key()=="brkxurl")
    if(kv.value().is<String>()) {
      g_cfg_brkxurl=kv.value().as<String>();
      ret=true;
    }        
    if(kv.key()=="brkxusr")
    if(kv.value().is<String>()) {
      g_cfg_brkxusr=kv.value().as<String>();
      ret=true;
    }        
    if(kv.key()=="brkxpwd")
    if(kv.value().is<String>()) {
      pwd=kv.value().as<String>();
      if(pwd!="?") g_cfg_brkxpwd=pwd;
      ret=true;
    }        
    if(kv.key()=="brkxtpc")
    if(kv.value().is<String>()) {
      g_cfg_brkxtpc=kv.value().as<String>();
      ret=true;
    }          
    if(kv.key()=="stassid")
    if(kv.value().is<String>()) {
      g_cfg_stassid=kv.value().as<String>();
      ret=true;
    }          
    if(kv.key()=="stapwd")
    if(kv.value().is<String>()) {
      pwd=kv.value().as<String>();
      if(pwd!="?") g_cfg_stapwd=pwd;
      ret=true;
    }          
    if(kv.key()=="devname")
    if(kv.value().is<String>()) {
      g_cfg_devname=kv.value().as<String>();
      ret=true;
    }          
    if(kv.key()=="opticnt")
    if(kv.value().is<int>()) {
      g_cfg_opticnt=kv.value().as<int>();
      ret=true;
    }          
  }
  return ret;
}
  

// conversion to JSON
void cfg_tojson(void) {
  g_jdocbuf.clear();
  g_jdocbuf["brkxen"] = g_cfg_brkxen;
  g_jdocbuf["brkxurl"] = g_cfg_brkxurl;
  g_jdocbuf["brkxusr"] = g_cfg_brkxusr;
  g_jdocbuf["brkxpwd"] = g_cfg_brkxpwd;
  g_jdocbuf["brkxtpc"] = g_cfg_brkxtpc;
  g_jdocbuf["stassid"] = g_cfg_stassid;
  g_jdocbuf["stapwd"]  = g_cfg_stapwd;
  g_jdocbuf["devname"] = g_cfg_devname;
  g_jdocbuf["opticnt"] = g_cfg_opticnt;
}

// load configuration
void cfg_load(void) {
  // read from  ESP32 EEPROM
  EEPROM.begin(JSONBUF);
  EEPROM.get(0,g_jstrbuf);
  EEPROM.end();
  // check for trailing '0'
  int p;
  for(p=0; p<JSONBUF; ++p)
    if(g_jstrbuf[p]==0) break;
  if(p==JSONBUF) {
    Serial.println("cfg_load: EEPROM not 0-terminated");
    Serial.println(g_jstrbuf);
    return;
  } 
  // deserialize
  if(deserializeJson(g_jdocbuf, g_jstrbuf)) {
    Serial.println("cfg_load: EEPROM not in JSON format");
    return;
  }
  // copy to global C++ vars
  if(!cfg_fromjson()) {
    Serial.println("cfg_load: no relevant entries in JSON record");
    return;
  }
  //success
  Serial.println("cfg_load: EEPROM loaded to conf OK");   
}


// save configuration
void cfg_save(void) {
  cfg_tojson();
  serializeJson(g_jdocbuf,g_jstrbuf); 
  Serial.println(String("cfg_save: ") + String(g_jstrbuf));
  EEPROM.begin(JSONBUF);
  EEPROM.put(0,g_jstrbuf);
  EEPROM.commit();
  EEPROM.end();
  // check back
  cfg_load();
}


/* 
**************************************************************************  
**************************************************************************  

Organize a collection of text-files in PROGMEM
- use the provided shell script "mkheaders.sh" to generate the 
  include files
  
**************************************************************************  
**************************************************************************  
*/ 

class ProgmemTextFiles {
public:

  // directory entry typedef
  typedef struct {
    const char*        name;
    const char*  text;
    const char*  type;
  } DirEntry;

  // seek file and prepare getters (return "true" on sucess)
  bool seek(String name) {
    fentry=NULL;
    fname="";
    ftype="";
    ftext=NULL;
    DirEntry* de=&directory[0];
    for(;de->name!=NULL;++de) 
      if(String(de->name)==name) break;
    if(de->name==NULL) return false;
    fname=name;
    ftype=String(de->type);
    ftext=de->text;
    fentry=de;
    return true;
  }    

  // get file details (call "seek(name)" before)
  String name(void) { return fname; }
  String type(void) { return ftype; }
  const char* text(void) { return ftext; }
  const DirEntry& direntry(void) { return *fentry; } 

private:
  DirEntry* fentry=NULL;
  String fname;
  String ftype;
  const char* ftext;  
  static DirEntry directory[]; 
};

// include actual text from external header files
#include "headers/index.html.h"
#include "headers/style.css.h"
#include "headers/power.svg.h"
#include "headers/question-circle.svg.h"
#include "headers/bootstrap-slider.min.js.h"
#include "headers/bootstrap-slider.min.css.h"
#include "headers/bootstrap.bundle.min.js.h"
#include "headers/bootstrap.min.css.h"
#include "headers/jquery.min.js.h"

// initialize directory
ProgmemTextFiles::DirEntry ProgmemTextFiles::directory[] = {
  { "index.html",               f_index_html,                "text/html"},
  { "style.css",                f_style_css,                 "text/css"},
  { "power.svg",                f_power_svg,                 "image/svg+xml"},
  { "question-circle.svg",      f_question_circle_svg,       "image/svg+xml"},
  { "jquery.min.js",            f_jquery_min_js,             "application/javascript"},
  { "bootstrap.min.css",        f_bootstrap_min_css ,        "text/css"},
  { "bootstrap.bundle.min.js",  f_bootstrap_bundle_min_js ,  "application/javascript"},
  { "bootstrap-slider.min.css", f_bootstrap_slider_min_css , "text/css"},
  { "bootstrap-slider.min.js",  f_bootstrap_slider_min_js ,  "application/javascript"},
  { NULL, NULL, NULL}
}; 

//Instantiate 
ProgmemTextFiles g_textfiles;




/* 
**************************************************************************  
**************************************************************************  

HTTP server for GET requests on PROGMEM text files
- specialize RequestHandler for our purpose
  
**************************************************************************  
**************************************************************************  
*/ 

// global server instance
WebServer  g_http_server(80);

// have HTTP GET handler to serve all GET requests 
class HttpGetHandler : public RequestHandler {
public:
  HttpGetHandler(void) {}

  bool canHandle(HTTPMethod method, String uri) override {
    if(method != HTTP_GET) return false;
    //Serial.println("testing HTTP-GET request for " + uri);
    if(uri=="/") uri = "/index.html";
    if(uri.indexOf("/")==0) uri.remove(0,1);
    if(!g_textfiles.seek(uri)) return false;
    return true;
  }
  
  bool handle(WebServer& server, HTTPMethod method, String uri) override {      
    if(method != HTTP_GET) return false;
    if(uri=="/") uri = "/index.html";
    if(uri.indexOf("/")==0) uri.remove(0,1);
    if(!g_textfiles.seek(uri)) {
      Serial.println("HTTP-GET request: \"" + uri + "\": file not found");
      return false;    
    }    
    Serial.println("HTTP-GET request: \'" + uri + "\": mtype \"" + g_textfiles.type() + "\" cnt #" + String(strlen(g_textfiles.text())));
    server.send_P(200, g_textfiles.type().c_str(), g_textfiles.text());    
    return true;
  }

};

// instantiate HTTP GET handler
HttpGetHandler* g_gethandler = new HttpGetHandler();

// install my handler and start the server
void http_init(void) {
  Serial.println("starting HTTP GET Server");
  g_http_server.addHandler(g_gethandler);
  g_http_server.begin();
}  

// loop callback
void http_lcb(void) {
  g_http_server.handleClient();
}  


/* 
**************************************************************************  
**************************************************************************  

WebSocket to talk to http client, i.e., our javascript code
- have appropruate event callback
  
**************************************************************************  
**************************************************************************  
*/ 

// global websocket server instance
WebSocketsServer g_wsck_server(8070);

// event callback (the current implementation replies by broadcast)
void websocket_ecb(uint8_t num, WStype_t type, uint8_t * data, size_t len) {
  IPAddress ip;
  DynamicJsonDocument jdoc(128);
  JsonObject jobj;
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("websocket [%u]: disconnected\n", num);
      break;
    case WStype_CONNECTED:
      ip = g_wsck_server.remoteIP(num);
      Serial.printf("websocket [%u]: connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], data);
      break;
    case WStype_TEXT:
      Serial.printf("websocket [%u]: got text: %s\n", num, data);
      if(deserializeJson(g_jdocbuf, data)) break;
      jobj=g_jdocbuf.as<JsonObject>();
      for(JsonPair kv : jobj) {
        // receive controls
        if(kv.key()=="smaxpow")
        if(kv.value().is<int>()) {
          g_tcontrol_smaxpow=kv.value().as<int>();
        }
        if(kv.key()=="onoff")
        if(kv.value().is<int>()) {
          g_tcontrol_sonoff=kv.value().as<int>();
        }        
        if(kv.key()=="avrcmd")
        if(kv.value().is<String>()) {
          g_tcontrol_vrbcmd=kv.value().as<String>();
        }        
        // receive parameter request
        if(kv.key()=="cmd") {
          String cmd=kv.value().as<String>();
          if(cmd=="cfgget") {
            g_wsck_server.broadcastTXT( String("{") +
              "\"brkxen\":"  + String(g_cfg_brkxen) + "," +
              "\"brkxurl\":\"" + g_cfg_brkxurl        + "\"," +
              "\"brkxusr\":\"" + g_cfg_brkxusr        + "\"," +
              "\"brkxtpc\":\"" + g_cfg_brkxtpc        + "\"," +
              "\"stassid\":\"" + g_cfg_stassid        + "\"," +
              "\"imaxpwr\":"   + String(g_tstate_imaxcur * 3.0 * 230.0 /1000.0,0) + "," +
              "\"smaxpwr\":"   + String(g_tstate_smaxcur * 3.0 * 230.0 /1000.0,0) + "," +
              "\"avrver\":\""  + verstr(g_tstate_version)  + "\"," +
              "\"espver\":\""  + verstr(VERSION)      + "\"," +
              "\"devname\":\"" + g_devname            + "\"}");
          }         
          if(cmd=="reset") {
            Serial.println("reset request");
            ESP.restart();
;          }               
        }
      } 
      // extract configuration, if any
      if(cfg_fromjson()) {
        Serial.printf("websocket [%u]: found config parameters\n",num);
        mqtt_config(); 
        if(g_cfg_devname!="???") 
          g_devname=g_cfg_devname;
        cfg_save();    
      }
      break;
    case WStype_BIN:
      Serial.printf("websocket [%u]: got data #%d\n", num, len);
      break;
    default:
      Serial.printf("websocket: [%u] ignoring event %d\n",num, (int) type);     
      break;
  }
} 

// debugging utility
void wsck_log(const String& msg) {
  String mgl(msg);
  mgl.replace('"','^');
  g_wsck_server.broadcastTXT("{\"diagln\":\"" + mgl +"\"}");
}

// send heartbeat message to all connected clients: autonomous
void wsck_send_heartbeat() {
  g_wsck_server.broadcastTXT(
    "{\"aphases\":"+String(g_tstate_aphases)+ ",\"amaxcur\":"+String(g_tstate_amaxcur)+",\"ccss\":"+String(g_tstate_ccss)+"}");
  g_wsck_server.broadcastTXT(
    "{\"cur1\":"+String(g_tstate_cur1)+",\"cur2\":"+String(g_tstate_cur2)+",\"cur3\":"+String(g_tstate_cur3)+"}");
  g_wsck_server.broadcastTXT(
    "{\"sphases\":"+String(g_tstate_sphases)+ ",\"smaxcur\":"+String(g_tstate_smaxcur)+",\"sonoff\":"+String(g_tstate_sonoff)+"}");
}    
  

// start websocher server and install our event callback
void wsck_init(void) {
  Serial.println("starting websocket server");
  g_wsck_server.begin();
  g_wsck_server.onEvent(websocket_ecb);
}

// loop callback
void wsck_lcb(void) {
  g_wsck_server.loop();
}  

/* 
**************************************************************************  
**************************************************************************  

MQTT client
  
**************************************************************************  
**************************************************************************  
*/ 

// instantiate MQT client
MQTTClient       g_mqtt_client;
Timer            g_mqtt_tcon;

// handler on measage receive
void mqtt_recv_cb(String &topic, String &payload) {
  Serial.println("mqttrecv: " + topic + " - " + payload);
}


// (re-)configure client {
void mqtt_config(void) {
  // parse ULR
  String dnsname;
  int port;
  int p;
  if(!g_cfg_brkxen) {
    Serial.println("external MQTT broker disabled");    
    return;
  }
  Serial.println("parsing configuration of external MQTT broker");
  dnsname=g_cfg_brkxurl;
  p=dnsname.indexOf("mqtt://");
  if(p>0) {
    Serial.println("error A parsing broker URL");
    return;
  }
  if(p==0) {
    dnsname=dnsname.substring(strlen("mqtt://"));
  }  
  p=dnsname.indexOf(":");
  if(p<0) {
    port=1883;
  }
  if( (p>0) && (dnsname.length()<= p+1)) {
    Serial.println("error B parsing broker URL");
    return;
  }  
  if(p>0) {
    port=dnsname.substring(p+1).toInt();
    dnsname=dnsname.substring(0,p);
  }  
  // set broker
  Serial.println("MQTT broker expected at "+String(dnsname)+":"+port);
  if(g_mqtt_client.connected())
    g_mqtt_client.disconnect();
  g_mqtt_client.setHost(dnsname.c_str(),port);
  g_mqtt_tcon.clear();
  // fix missing leading "/" in topic
  if(g_cfg_brkxtpc.indexOf("/")!=0)
    g_cfg_brkxtpc= "/"+g_cfg_brkxtpc;
}

// start client
void mqtt_init() {
  if(!g_stamode) return;
  Serial.println("starting MQTT client");
  g_mqtt_client.begin(g_network);
  g_mqtt_client.onMessage(mqtt_recv_cb);
  mqtt_config();
} 

// manage connection in loop call back
void mqtt_lcb(void) {
  if(!g_stamode) return;
  // maintain mqqt libraray
  g_mqtt_client.loop();
  // manage connection to broker 
  if((!g_mqtt_client.connected()) && (g_cfg_brkxen) && (!g_mqtt_tcon.running())) { 
    Serial.println("MQTT client trying to connect to broker");
    g_mqtt_client.connect(g_devname.c_str());
    g_mqtt_tcon.set(5000); 
  }  
  if(g_mqtt_client.connected()) 
    g_mqtt_tcon.clear();     
  // publish heartbeat
  static Periodic phb(5000,100);
  if(g_mqtt_client.connected() && phb.expired()) {
    Serial.println("MQTT client publishing heartbeat");
    g_mqtt_client.publish(g_cfg_brkxtpc+"/heartbeat",  String("{") +
        "\"cur1\":"   +String(g_tstate_cur1)   +","+
        "\"cur2\":"   +String(g_tstate_cur2)   +","+
        "\"cur3\":"   +String(g_tstate_cur3)   +","+
        "\"amaxcur\":"+String(g_tstate_amaxcur)+"}");
  }
}


/* 
**************************************************************************  
**************************************************************************  

Target AVR Control
- this is out pragmatic payload
- sense g_tcontrl_* variables and send coommands to the target 
- query target state periodically to update g_tstate_* variables
  
**************************************************************************  
**************************************************************************  
*/ 

// if no target ist present, we simulate some random behaviour for 
// testing/development e.g. on an M5StickC
#ifdef AVRSIMULATE

bool  g_tstate_update=false;


void target_init();

void  target_lcb() {
  // remote on/off command, exec in 5 secs
  static Timer tonoff;
  static int vccss;
  if( (g_tcontrol_onoff==0) && (g_tstate_ccss >= 10) ) { 
    tonoff.set(5000);
    vccss=0;
  }    
  if( (g_tcontrol_onoff==1) && (g_tstate_ccss < 10) ) {
    tonoff.set(5000);
    vccss=10;
  }    
  g_tcontrol_onoff=-1;
  if(tonoff.expired()) {
    g_tstate_ccss=vccss;
  }  
  // remote set max power, exec in 5 secs
  static Timer tpowset;
  static int vsmaxcur;
  if(g_tcontrol_smaxpow>=0) {
    tpowset.set(5000);
    vsmaxcur=g_tcontrol_smaxpow*100.0 / (230.0 * 3) *10 +0.5; 
  }   
  g_tcontrol_smaxpow=-1;
  if(tpowset.expired()) {
    if(vsmaxcur>g_tstate_imaxcur) 
      vsmaxcur = g_tstate_imaxcur;
    g_tstate_smaxcur=vsmaxcur;
    g_tstate_amaxcur=vsmaxcur;
  }   
  // remote set installed max power, exec instantly
  if(g_tcontrol_imaxpow>=0) {
      g_tstate_imaxcur = g_tcontrol_imaxpow*100.0 / (230.0 * 3) *10 +0.5; 
      if(g_tstate_smaxcur>g_tstate_imaxcur) 
        g_tstate_smaxcur = g_tstate_imaxcur;
  }   
  g_tcontrol_imaxpow=-1;     
  // simulate funny state variables
  g_tstate_update=false;
  static Periodic psim(5000);
  if(psim.expired()) {
    g_tstate_ccss+=1;
    g_tstate_cur1+=7;
    g_tstate_cur2+=23;
    g_tstate_cur3+=34;
    if(g_tstate_ccss>105) g_tstate_ccss=-3;    
    if(g_tstate_cur1>g_tstate_amaxcur) g_tstate_cur1=-1;    
    if(g_tstate_cur2>g_tstate_amaxcur) g_tstate_cur2=0;    
    if(g_tstate_cur3>g_tstate_amaxcur) g_tstate_cur3=0; 
    g_tstate_update=true;
  }
}

#endif


// talk to actual target AVR via UART_NUM_2
#ifdef AVRSERIAL

bool g_tstate_update=false;

#define READLNBUF 128

static String g_readln_buf;

void target_init(void) {
  while(Serial2.available()>0) Serial2.read();
}

void  target_lcb() {  
  // line status
  static boolean lnbsy=(Serial2.availableForWrite()<20);
  static boolean wtrply=false;
  // sense power on
  static Timer pot(5000);
  static bool pon=true;
  if(pot.expired()) pon=false;
  // assemble line from serial
  char c=0;
  int  n=0;
  while((Serial2.available()>0) && (c!='\n') && (n<64)) {
    c=Serial2.read();
    n++;
    if(g_readln_buf.length()<READLNBUF) 
      g_readln_buf.concat(c);
  }
  // sense eol: remove trailing "\r\n"
  if(c=='\n') {
    if(g_readln_buf.endsWith("\n")) 
      g_readln_buf.remove(g_readln_buf.length()-1);
    if(g_readln_buf.endsWith("\r")) 
      g_readln_buf.remove(g_readln_buf.length()-1);
  }
  // sense eol: swallow comments
  if(c=='\n') {
    if(g_readln_buf.startsWith("%")) {
      wsck_log(g_readln_buf);
      c=0;
      g_readln_buf="";      
    }
  }
  // sense eol: figure position of name and value
  int bg=-1;
  int nm=-1;
  int sg=1;
  if(c=='\n') {
    for(bg=0; bg<g_readln_buf.length(); ++bg)
      if(g_readln_buf.charAt(bg) != '[') break;
    if(bg>=g_readln_buf.length()) bg=-1;
  }
  if(bg>=0) {    
    nm=g_readln_buf.indexOf("=",bg)+1;
    if((nm>0) && (nm<g_readln_buf.length())) {
      for(;nm<g_readln_buf.length();++nm)
        if(!isSpace(g_readln_buf.charAt(nm))) break;
      if(nm<g_readln_buf.length()) 
        if(g_readln_buf.charAt(nm)=='-') {sg=-1; nm++;} 
      if(nm<g_readln_buf.length()) {
        if(!isDigit(g_readln_buf.charAt(nm)))
          nm=g_readln_buf.length();
      }
    }
    if(nm>=g_readln_buf.length()) nm=-1;
  }
  // sense eol: extract parname/parvalue  
  String pn="";
  int pv=-1;
  if((bg>=0) && (nm>bg)) {
    pn=g_readln_buf.substring(bg, sg==1 ? nm-1 : nm-2);
    pv= sg * g_readln_buf.substring(nm).toInt();
  }   
  // clear buffer
  if(c=='\n') {
    String msg;
    if(pn!="")
      msg="serial (prs): [" + pn + "=" + pv + "] @(" + bg +","+nm+")";
    else 
      msg="serial (prs): [parse error] @(" + g_readln_buf +")";
    Serial.println(msg);
    //wsck_log(msg);
    g_readln_buf="";
    wtrply=false;
  }  
  // update local copy of state
  g_tstate_update=false;
  if(pn!="") {
    if(pn=="ver") g_tstate_version=pv; else
    if(pn=="ccss") g_tstate_ccss=pv; else
    if(pn=="sphases") g_tstate_sphases=pv; else
    if(pn=="aphases") g_tstate_aphases=pv; else
    if(pn=="imaxcur") g_tstate_imaxcur=pv; else
    if(pn=="smaxcur") g_tstate_smaxcur=pv; else
    if(pn=="amaxcur") g_tstate_amaxcur=pv; else
    if(pn=="cmaxcur") g_tstate_cmaxcur=pv; else
    if(pn=="cur1") g_tstate_cur1=pv; else
    if(pn=="cur2") g_tstate_cur2=pv; else
    if(pn=="cur3") g_tstate_cur3=pv; else
    wsck_log(pn+"="+pv); 
    g_tstate_update=true;
  }
  // send power-on requests
  if(pon && (!lnbsy) && (!wtrply)) {  
    static Periodic b0(1000,     0);
    if(b0.expired()) { Serial2.println("ver?"); wtrply=true;} 
    static Periodic b1(1000,  500);
    if(b1.expired()) { Serial2.println("imaxcur?"); wtrply=true;}  
  }
  // clear on/off
  if(g_tstate_ccss<10) {
    if(g_tstate_sonoff==0) g_tcontrol_sonoff=1;
  }
  // forward commands
  if((g_tcontrol_sphases>=0) && (!lnbsy) && (!wtrply)) {  
    Serial2.println("sphases="+String(g_tcontrol_sphases));
    g_tcontrol_sphases=-1;
    wtrply=true;
  }
  if((g_tcontrol_smaxpow>=0) && (!lnbsy) && (!wtrply)) {  
    g_tstate_smaxcur= (int) (g_tcontrol_smaxpow*100.0 / (230.0 * 3) *10 +0.5);
    if(g_tstate_sonoff) 
       Serial2.println("smaxcur="+String(g_tstate_smaxcur));
     else  
       Serial2.println("smaxcur=0");      
    g_tcontrol_smaxpow=-1;
    wtrply=true;
  }
  if((g_tcontrol_sonoff>=0) && (!lnbsy) && (!wtrply)) {  
    if((g_tcontrol_sonoff==0) && (g_tstate_ccss >=10) && (g_tstate_ccss <50))
      g_tstate_sonoff=g_tcontrol_sonoff;
    if((g_tcontrol_sonoff==1) && (g_tstate_ccss >=50) && (g_tstate_ccss <60))
      g_tstate_sonoff=g_tcontrol_sonoff;
    if(g_tstate_sonoff) 
       Serial2.println("smaxcur="+String(g_tstate_smaxcur));
     else  
       Serial2.println("smaxcur=0");      
    g_tcontrol_sonoff=-1;
    wtrply=true;
  }
  if((g_tcontrol_imaxpow>=0) && (!lnbsy) && (!wtrply)) {  
    Serial2.println("imaxcur="+String((int) (g_tcontrol_imaxpow*100.0 / (230.0 * 3) *10 +0.5) ));
    g_tcontrol_imaxpow=-1;
    wtrply=true;
  }
  if((g_tcontrol_vrbcmd!="") && (!lnbsy) && (!wtrply)) {   
    Serial2.println(g_tcontrol_vrbcmd);
    g_tcontrol_vrbcmd="";
    wtrply=true;
  }
  // send periodic state update
  if( (!pon) && (!lnbsy) && (!wtrply)) {  
    static Periodic p0(1000,     0);
    if(p0.expired()) { Serial2.println("ccss?"); wtrply=true;} 
    static Periodic p1(5000,  500);
    if(p1.expired()) { Serial2.println("aphases?"); wtrply=true;}  
    static Periodic p2(5000,  1500);
    if(p2.expired()) { Serial2.println("amaxcur?"); wtrply=true;} 
    static Periodic p3(5000,  2500);
    if(p3.expired()) { Serial2.println("cur1?"); wtrply=true;} 
    static Periodic p4(5000,  3500);
    if(p4.expired()) { Serial2.println("cur2?"); wtrply=true;}  
    static Periodic p5(5000,  4500);
    if(p5.expired()) { Serial2.println("cur3?"); wtrply=true;} 
  }
}
#endif


/* 
**************************************************************************  
**************************************************************************  

Target AVR firmware update
- update AVR flash from ESP32 PROGMEN, i.e. we get the acrtual target
  firmware by ESP32 OTA and flash the target AVR it at restart
- this is very simplistic implementation of an Optiboot counterpart;
  for principle of operation, see AVRs documentation of the STK500
  protocol
- this is sychronous; so dont call from the main loop
  
**************************************************************************  
**************************************************************************  
*/ 

#ifdef AVRUPDATE
#include "headers/ctrl22c.bin.h"

class OptiFlash {
public:
  // construct
  OptiFlash(void) {}
  // configure
  void image(const unsigned char* img, unsigned int cnt)  {fimg=img; fcnt=cnt;}
  void toffset(unsigned int off=0x200) {toff=off;}
  void tbyteaddr(bool b=false) {baddr=b;}
  void tserial(HardwareSerial& ser) {avrser=ser;}
  // ustils halt/run avr 
  void avrhalt(void) { digitalWrite(GPIORST,LOW); }
  void avrrun(void) { digitalWrite(GPIORST,HIGH); }
private:
  // conf
  static const int buflen=128+10;
  static const int pagelen=128;
  HardwareSerial& avrser=Serial2;     // serial for flashing the AVR
  const unsigned char* fimg;          // data src 
  unsigned int fcnt;                  // data length
  unsigned int toff;                  // target offset
  boolean baddr;                      // true for byte address mode (nonstd.)  
  // state
  char iobuf[buflen];  // r/w buffer
  int ioerr=0x0;       // accumulative error flags
  // utils: send a command incl sync
  void sndcmd(int scnt, int rcnt) {
    while(avrser.available()) avrser.read();  // flush UART
    iobuf[scnt++]=0x20;                       // add opcode "Sync_EOP"
    rcnt++;                                   // expect reply on "Sync_EOP"
    //Serial.println("optiflash: sending #"+String(scnt)+ " (0x"+String(iobuf[0],HEX)+")");
    avrser.write(iobuf,scnt);                // synchronous write
    delay(2+scnt);
    memset(iobuf,0,buflen);                   // cosmetic (good for error codes)
    avrser.setTimeout(100);                   // 100ms timeout
    if((scnt=avrser.readBytes(iobuf,rcnt))!=rcnt) ioerr|= 0x0001;
    //Serial.println("optiflash: read #"+String(scnt)+ " ("+String(iobuf[0],HEX)+")");
    if(iobuf[0]!=0x14) ioerr|=0x0002;         // expect sync reply
    return;
  }
  // utils: sync request
  void optsync(void) {
    iobuf[0]=0x30; // op-code "sync request"
    sndcmd(1,1);
    if(iobuf[1]!=0x10) ioerr|= 0x0004;
  }
  // utils: veryfy presence of Optiboot
  void optver(void) {
    int min;
    int maj;
    iobuf[0]=0x41;                             // opcode "read parameter"
    iobuf[1]=0x81;                             // major version
    sndcmd(2,2);
    maj=iobuf[1];
    if(iobuf[2]!=0x10) ioerr|= 0x0008;
    iobuf[0]=0x41;                             // opcode "read parameter"
    iobuf[1]=0x82;                             // minor version
    sndcmd(2,2);
    min=iobuf[1];
    if(iobuf[2]!=0x10) ioerr|= 0x0008;
    if(ioerr==0x0)
      Serial.println("optiflash: found version " + String(maj) + "." + String(min));
  }
  // utils: load target address (nonstd: we use byte address, see "demesh.c")
  void optaddr(unsigned int addr) {
    iobuf[0]=0x55;                            // opcode "set address"
    addr+=toff;                               // target offset
    if(!baddr) addr /= 2;                     // addressing mode (std is word address)
    iobuf[1]= (unsigned char) (addr & 0xff);  // low byte of address
    iobuf[2]= (unsigned char) (addr  >> 8);   // high byte of address
    sndcmd(3,1);
    if(iobuf[1]!=0x10) ioerr|=0x0010;
  }
  // utils: flash one page
  void optwrite(unsigned int addr) {
    iobuf[0]=0x64;            // opcode "write page"
    iobuf[1]= pagelen >> 8;   // high byte of byte count
    iobuf[2]= pagelen & 0xff; // low byte of byte count
    iobuf[3]= 0x46;           // opcode "destination is flash"
    memcpy(iobuf+4, fimg+addr, pagelen);  
    sndcmd(pagelen+4,1);     // do send
    if(iobuf[1]!=0x10) ioerr|=0x0020;
  }     
  // utils: verify flash page
  void optread(unsigned int addr) {
    iobuf[0]=0x74;                // opcode "read page"
    iobuf[1]= pagelen >> 8;   // hight byte of byte count
    iobuf[2]= pagelen & 0xff; // low byte of byte count
    iobuf[3]= 0x46;               // opcode "source is flash"
    sndcmd(4,pagelen+1);
    if(iobuf[pagelen+1]!=0x10) ioerr|=0x0040;
    if(memcmp(fimg+addr,iobuf+1,pagelen)!=0) ioerr|=0x0080;
  }
  // utils: quit to main app
  void optquit(void) {
    iobuf[0]=0x51; // opcode "exit to application programm"
    sndcmd(1,1);
    if(iobuf[1]!=0x10) ioerr|=0x0100;
  }
public:
  // payload: do login to optioboot
  int doopti(void) {
    // reset target
    Serial.println("optiflash: reset target");
    avrhalt();
    delay(500);
    avrrun();
    delay(200);
    // sync with optiboot
    Serial.println("optiflash: synchronizing");
    int fail=20;
    for(; fail>0; fail--) {
      ioerr=0x0;
      optsync();
      if(ioerr==0x0) break;
      delay(100);
    }
    if(ioerr!=0) {
      Serial.println("optiflash: failed to sync");
      return -1;
    }
    // test for STK500 verion
    optver();
    if(ioerr!=0) {
      Serial.println("optiflash: failed to read STK500 version");
      return -1;
    }
    return 0;
  }
  // payload: do flash
  int doflash(void) {
    // say hello
    Serial.println("optiflash: do flash -- cnt " +String(fcnt));
    // enter optiboot
    if(doopti()!=0) return -1;
    // flash
    int fail=20;
    for(unsigned int addr=0; addr<fcnt; addr+=pagelen) {
      Serial.println("optiflash: writing to flash at "+String(addr,HEX));
      for(; fail>0; fail--) {
        ioerr=0x0;
        optaddr(addr);
        optwrite(addr);
        if(ioerr==0x0) break;
        delay(100);
      }
      if(ioerr!=0x0) {
        Serial.println("optiflash: failed writing  at "+String(addr,HEX));
        return -1;
      }
    }
    // done
    optquit();
    if(ioerr!=0x0) {
      Serial.println("optiflash: failed to restart target");
      return -1;
    }
    return 0;
  }
  // payload: do verify
  int doverify(void) {
    // say hello
    Serial.println("optiflash: do verify -- cnt " +String(fcnt));
    // enter optiboot
    if(doopti()!=0) return -1;
    // verify
    int fail=20;
    for(unsigned int addr=0; addr< fcnt; addr+=pagelen) {
      Serial.println("optiflash: reading flash at "+String(addr,HEX));
      for(; fail>0; fail--) {
        ioerr=0x0;
        optaddr(addr);
        optread(addr);
        if(ioerr==0x0) break;
        delay(100);
      }
      if(ioerr!=0x0) {
        Serial.println("optiflash: verification failed at "+String(addr,HEX) + 
          " with err "+String(ioerr,HEX));
        if(ioerr==0x0080) return -2;  
        return -1;
      }
    }
    // quit optiboot
    optquit();
    if(ioerr!=0x0) {
      Serial.println("optiflash: failed to restart target");
      return -1;
    }
    return 0;
  }
};

// convenience wrapper for std startup
void avrflash() {
  // have OptiFlash configure
  OptiFlash ofl;
  ofl.image((const unsigned char*) f_ctrl22c_bin, f_ctrl22c_bin_len);
  ofl.toffset(0x200);
  ofl.tbyteaddr(true);
  ofl.tserial(Serial2);
  // verify image
  int res = ofl.doverify();
  if(res==-1) {
    Serial.println("optiflash: fatal error, can not verify target avr flash");
    ofl.avrhalt();
    return;
  }
  if(res==0)  {
    Serial.println("optiflash: target avr flash ok");
    if(g_cfg_opticnt!=0) {
      g_cfg_opticnt=0;
      cfg_save();
    }
    return;
  }  
  if(g_cfg_opticnt<3) {
    Serial.println("optiflash: verification failed, re-flashing target firmware");
    ofl.doflash();
    g_cfg_opticnt++;
    cfg_save();
    Serial.println("optiflash: re-starting alltogether");
    ESP.restart();
  }  
  Serial.println("optiflash: fatal error, invalid avr flash, max count reached");
  ofl.avrhalt();    
}  


#endif

/* 
**************************************************************************  
**************************************************************************  

Std Arduino sketch with "setup()" and "loop()"
  
**************************************************************************  
**************************************************************************  
*/ 

// std Arduino initialization
void setup() {

  // init my hardware
  dev_init();
   
  // serial line report/debug
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println("AGCCS-CTRLONE (firmware v"+ String(VERSION/10) +"." + String(VERSION %10)+")");

  // read configuration from EEPROM
  cfg_load();  

  // figure my device name by MAC
  unsigned char mac[6];
  WiFi.macAddress(mac);
  g_devname="agccs-";
  for(int i=0; i<6;++i) 
    g_devname += String(mac[i],HEX); 
  Serial.println("factory name: " +g_devname);

  // overwrite by configuration
  if(g_cfg_devname!="???") 
    g_devname=g_cfg_devname;
 
  // set our device name as hostname
  WiFi.setHostname(g_devname.c_str());

  // try to connect in station mode (10secs)
  g_stamode=false;
  WiFi.mode(WIFI_STA);
  Serial.print("attempting to connect to \""+g_cfg_stassid+"\"");
  WiFi.begin(g_cfg_stassid.c_str(), g_cfg_stapwd.c_str());
  for(int cnt=0;cnt<100;++cnt) {
    g_stamode= (WiFi.status() == WL_CONNECTED);
    if(g_stamode) break;
    Serial.print(".");
    delay(100);
  }  
  if(g_stamode) Serial.println("ok"); 
  else Serial.println("fail");

  // fal back to provide AP
  if(!g_stamode) {
    WiFi.mode(WIFI_AP);
    Serial.println("starting AP "+g_devname+ " (pwd: \"" + g_cfg_sappwd +"\")");
    WiFi.softAP(g_devname.c_str(), g_cfg_sappwd.c_str());
  }  

  // report IP address
  IPAddress ip= WiFi.localIP();
  Serial.print("IP address ");  
  Serial.println(ip);  

  // install my tcp servers/clients
  wsck_init();
  mqtt_init();
  http_init();

#ifdef AVRUPDATE
  avrflash();
#endif

  // enable OTA
  ArduinoOTA.setHostname(g_devname.c_str());
  ArduinoOTA.setPassword("ctrl22one");
  ArduinoOTA.begin();

  // prepare serial connection to tagert
  target_init();
}


// std Arduino infinite loop
void loop() {

  // maintaine systime
  systime_lcb();

  // development heartbeat
  static Periodic phb(10000);
  if(phb.expired()) {
    Serial.println("systime: "+String(g_systime/1000)+" [sec]");
    wsck_log("systime: "+String(g_systime/1000)+" [sec]");
  }

  // blink
  static Periodic pon(2000,   0);
  if(pon.expired()) led_on();
  static Periodic poff(2000, 50);
  if(poff.expired()) led_off();

  // excercise target AVR control 
  target_lcb();

  // forward local copy of AVR state to websocket client aka browser       
  if(g_tstate_update) 
    wsck_send_heartbeat();
    
  // loop maintenance
  http_lcb();
  wsck_lcb();
  mqtt_lcb();  
  ArduinoOTA.handle();
  delay(5);
}
