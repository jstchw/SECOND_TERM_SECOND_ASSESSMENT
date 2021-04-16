#include <Arduino.h>
#include <Wire.h> // WIRE LIBRARY
#include <WifiClient.h> // LIBRARY TO START THE AP
#include <ESP8266WebServer.h> // LIBRARY TO START THE WEB SERVER
#include <ESP8266WebServerSecure.h> // LIBRARY TO START THE SECURE WEB SERVER
#include "espPages.h"

/* === WEB PAGES === */
const char* ssid = "Do not touch me";
const char* password = "6zJS6GB5";
int status_code;
BearSSL::ESP8266WebServerSecure serverHTTPS(443);
ESP8266WebServer serverHTTP(80);
IPAddress ipaddr;

/* === HANDSHAKING LINES MACROS === */
#define DEMAND D1
#define GRANTED D2

/* === LED MACROS === */
#define LED D4
#define LED_ON digitalWrite(LED, LOW);
#define LED_OFF digitalWrite(LED, HIGH);

/* === RESOURCE MANAGER MACROS === */
#define NoTriggerNotGranted 0 // IDLE STATE
#define TriggerNotGranted 1 // TRIGGER DETECTED
#define TriggerIamMaster 2 // ESP IS A MASTER (ESP CANNOT BE A SLAVE)

/* === RESOURCE MANAGER VARIABLES === */
int managerState;
bool trigger;

/* === MODULE VARIABLES === */
bool init_module0_clock, init_module2_clock;

/* === WEB VARS === */
unsigned char status;
bool receiveFlag;

/* === FUNCTION PROTOTYPES === */
bool granted();
void leaveHigh(unsigned char pin);
void pullLow(unsigned char pin);
void handleRoot();
void handleSubmit();
void redirectHTTPS();
void sendData(String arg);
void receiveEvent(int num);
void requestEvent();
void sendRespond();
void getAck();

void setup() {
  /* === RESOURCE MANAGER SETUP === */
  leaveHigh(DEMAND);
  pinMode(GRANTED, INPUT_PULLUP);

  /* === RESOURCE MANAGER SETUP === */
  managerState = NoTriggerNotGranted;
  trigger = false;

  init_module0_clock = true;
  init_module2_clock = true;

  /* === LED SETUP === */
  pinMode(LED, OUTPUT);

  /* === WEB SETUP === */
  WiFi.softAP(ssid, password);

  // SECURE SERVER NORMAL MODE
  serverHTTPS.on("/", handleRoot);
  serverHTTPS.begin();

  // INSECURE SERVER REDIRECT MODE
  serverHTTP.on("/", redirectHTTPS);
  serverHTTP.begin();

  status_code = 0;
  ipaddr = WiFi.softAPIP();

  // SSL CERTIFICATE
  serverHTTPS.getServer().setRSACert(new BearSSL::X509List(SERVER_CERT), new BearSSL::PrivateKey(SERVER_KEY));

  /* === OVERCLOCKING === */
  system_update_cpu_freq(160);

  /* === SERIAL MONITOR SETUP === */
  Serial.begin(9600);

  /* === WIRE LIB SETUP === */
  Wire.begin(D6, D7);
  //Wire.begin(8);
  //Wire.begin(9);
  Wire.onReceive(receiveEvent);
  //Wire.onRequest(requestEvent);
  receiveFlag = false;
}

void loop() {
  switch(managerState) {
    case NoTriggerNotGranted:
      //Serial.print("DEMAND: HIGH ");
      //Serial.println("NoTriggerNotGranted");
      leaveHigh(DEMAND);
      if(!trigger) {
        managerState = NoTriggerNotGranted;
      }
      if(trigger && granted()) {
        managerState = TriggerIamMaster;
      }
      if(trigger && !granted()) {
        managerState = TriggerNotGranted;
      }
      break;

    case TriggerNotGranted:
      pullLow(DEMAND);
      if(trigger && !granted()) {
        managerState = TriggerNotGranted;
      }
      if(trigger && granted()) {
        managerState = TriggerIamMaster;
      }
      if(!trigger) {
        managerState = NoTriggerNotGranted;
      }
      break;

    case TriggerIamMaster:
      pullLow(DEMAND);
      if(trigger && granted()) {
        managerState = TriggerIamMaster;
      }
      if(!(trigger && granted())) {
        managerState = NoTriggerNotGranted;
      }
      break;
    default:
      managerState = NoTriggerNotGranted;
      break;
  }

  /* === MODULE 0 - LED MASTER INDICATOR === */

  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    unsigned long timeStamp;
    
    if (init_module0_clock) {
      module_delay = 10;
      module_time = millis();
      module_doStep = false;
      init_module0_clock = false;
      state = 0;
      timeStamp = 0;
    }
    else {
      unsigned long m = millis();
      if (((unsigned long)(m - module_time)) > module_delay) {
        module_time = m; 
        module_doStep = true;
      }
      else module_doStep = false;
    }

    if (module_doStep) {
      switch(state) {
        case 0:
          LED_OFF;
          timeStamp = millis();
        case 1:
          if(((unsigned long)(millis() - timeStamp)) < 750) {
            state = 1;
          }
          else {
            trigger = true;
            state = 2;
          }
          break;
        case 2:
          if(managerState == TriggerIamMaster) {
            state = 3;
          }
          else {
            state = 2;
          }
          break;
        case 3:
          //requestStatus();
          //if (receiveFlag) Serial.println("1");
          //Serial.println(status + "Arduino");
          //sendRespond();
          //Serial.println("Status Requested...");
          //getAck();
          LED_ON;
          trigger = false;
          state = 0;
          break;

          default: 
          state = 0;
          break;
      }  
    }
  }

  /* === MODULE 1 - WEB SERVER === */

  {
    static unsigned long module_time = millis(), module_delay = 0;
    
      if (((unsigned long)(millis() - module_time)) > module_delay) {
        module_time += module_delay;

        serverHTTPS.handleClient();
        serverHTTP.handleClient();
      }
  }
}

/* === FUNCTIONS === */
bool granted() {
  return (!digitalRead(GRANTED));
}

void leaveHigh (unsigned char pin) {
  pinMode(pin, INPUT_PULLUP);
}

void pullLow (unsigned char pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

/* === WEB FUNCTIONS === */

void handleRoot() {
  if(serverHTTPS.hasArg("traffic") || serverHTTPS.hasArg("tri") || serverHTTPS.hasArg("fruit")) {
    handleSubmit();
    serverHTTPS.send(200, "text/html", INDEX_HTML);
  }
  else {
    serverHTTPS.send(200, "text/html", INDEX_HTML);
  }
}

void handleSubmit() {
  String arg;

  if(!serverHTTPS.hasArg("traffic") && !serverHTTPS.hasArg("fruit") && !serverHTTPS.hasArg("tri")) {
    return;
  }

  if(serverHTTPS.hasArg("traffic")) {
    arg = serverHTTPS.arg("traffic");
    sendData(arg);
    arg.clear();
  }

  if(serverHTTPS.hasArg("tri")) {
    arg = serverHTTPS.arg("tri");
    sendData(arg);
    arg.clear();
  }

  if(serverHTTPS.hasArg("fruit")) {
    arg = serverHTTPS.arg("fruit");
    sendData(arg);
    arg.clear();
  }
}

void sendData(String arg) {
  Wire.beginTransmission(8);

  if(arg == "traffic_a") Wire.write(0x61);
  else if(arg == "traffic_b") Wire.write(0x62);
  else if(arg == "traffic_c") Wire.write(0x63);
  else if(arg == "traffic_d") Wire.write(0x60);

  if(arg == "tri_a") Wire.write(0x65);
  else if(arg == "tri_b") Wire.write(0x66);
  else if(arg == "tri_c") Wire.write(0x67);
  else if(arg == "tri_d") Wire.write(0x68);
  else if(arg == "tri_e") Wire.write(0x69);

  if(arg == "fruit_a") Wire.write(0x60);
  else if(arg == "fruit_b") Wire.write(0x6A);


  Wire.endTransmission(true);
  Serial.println("Arg sent: " + arg);
  getAck();
}

void requestEvent() {

}

void receiveEvent(int num) {
  while(0 < Wire.available()) {
    status = Wire.read();
  }
  receiveFlag = true; 
}

void sendRespond() {
  if(receiveFlag) {
    Wire.beginTransmission(8);
    if(status == 0x41) {
      Wire.write(0x60); //Confirmation (change idle state from 0x60 to 0x59 NANO)
      Serial.println("Confirmation sent to Nano");
    }
    receiveFlag = false;
    Wire.endTransmission(true);
  }
}

void getAck() {
  Wire.requestFrom(8, 1); //Requesting ack or nack from nano
  while(0 < Wire.available()) {
    status = Wire.read();
  }
  Serial.println(status);
  /*if(status == 0x41) {
    Wire.beginTransmission(8);
    Wire.write(0x41); //ACK from ESP to NANO
    Wire.endTransmission(true);
  }*/
}



void redirectHTTPS() {
  serverHTTP.sendHeader("Location", String("https://") + ipaddr.toString(), true);
  serverHTTP.send(301, "text/plan", "");
}
