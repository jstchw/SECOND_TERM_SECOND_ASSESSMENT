#include <Arduino.h>
#include <Wire.h>

/* === HANDSHAKING LINES MACROS === */
#define DEMAND D1
#define GRANTED D2

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

  managerState = NoTriggerNotGranted;
  trigger = false;

  /* === SERIAL MONITOR SETUP === */
  Serial.begin(9600);

  /* === WIRE LIB SETUP === */
  Wire.begin(D6, D7);
}

void loop() {
  switch(managerState) {
    case NoTriggerNotGranted:
      Serial.print("DEMAND: HIGH ");
      Serial.println("NoTriggerNotGranted");
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
      Serial.print("DEMAND: LOW ");
      Serial.println("TriggerNotGranted");
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
      Serial.print("DEMAND: LOW ");
      Serial.println("TriggerIamMaster");
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