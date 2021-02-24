#include <Arduino.h>
#include <Wire.h>

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
bool init_module0_clock;

/* === FUNCTION PROTOTYPES === */
bool granted();
void leaveHigh(unsigned char pin);
void pullLow(unsigned char pin);

void setup() {
  /* === RESOURCE MANAGER SETUP === */
  leaveHigh(DEMAND);
  pinMode(GRANTED, INPUT_PULLUP);

  /* === RESOURCE MANAGER SETUP === */
  managerState = NoTriggerNotGranted;
  trigger = false;

  init_module0_clock = true;

  /* === LED SETUP === */
  pinMode(LED, OUTPUT);

  /* === SERIAL MONITOR SETUP === */
  Serial.begin(9600);

  /* === WIRE LIB SETUP === */
  Wire.begin(D6, D7);
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
      //Serial.print("DEMAND: LOW ");
      //Serial.println("TriggerNotGranted");
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
      //Serial.print("DEMAND: LOW ");
      //Serial.println("TriggerIamMaster");
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
      module_delay = 1;
      module_time = millis();
      module_doStep = false;
      init_module0_clock = false;
      state = 0;
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
          //Serial.println("SWITCH I: CASE 0");
          LED_OFF;
          timeStamp = millis();
        case 1:
          //Serial.println("SWITCH I: CASE 1");
          if(((unsigned long)(millis() - timeStamp)) < 15) {
            state = 1;
          }
          else {
            trigger = true;
            state = 2;
          }
          break;
        case 2:
          //Serial.println("SWITCH I: CASE 2");
          if(managerState == TriggerIamMaster) {
            state = 3;
          }
          else {
            state = 2;
            //Serial.println(state);
            //Serial.println(TriggerIamMaster);
          }
          break;
        case 3:
          //Serial.println("SWITCH I: CASE 3");
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