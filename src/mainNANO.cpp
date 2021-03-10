#include <Arduino.h>
#include <Wire.h>

/* === UI MACROS === */
#define _B0 13
#define _B1 A1
#define _B2 A2

/* === SHIFT REGISTER MACROS === */
#define dataPin 8
#define latchPin A3
#define clockPin 12

/* === IO MACROS === */
#define GRANTED A0
#define ANALOG_INPUT A6

/* === MPU MACROS === */
#define MPU_addr 0x68
#define ACCEL_XOUT_H 0x3B
#define TEMP_OUT_H 0x41

/* === RESOURCE MANAGER MACROS === */
#define NoTriggerNoDemand 0 // IDLE STATE
#define TriggerNoDemand 1 // TRIGGER DETECTED
#define TriggerIamMaster 2 // NANO IS A MASTER
#define IamSlave 3 // NANO IS A SLAVE

/* === LED MACROS === */
#define LED_RED_1  2 // RED 1st row
#define LED_YELLOW_1 3 // YELLOW 1st row
#define LED_GREEN_1 4 // GREEN 1st row

#define LED_RED_2  5 // RED 2nd row
#define LED_YELLOW_2 6 // YELLOW 2nd row
#define LED_GREEN_2 7 // GREEN 2nd row

#define LED_TRI_R 9
#define LED_TRI_B 10
#define LED_TRI_G 11

/* === BUTTON STATES MACROS === */
#define notPRESSED 0
#define partialPRESS 1
#define normalPRESS 2
#define debounce 300 // DEBOUNCE DURATION

/* === RESOURCE MANAGER VARIABLES === */
int managerState;
bool trigger;

/* === BUTTON VARIABLES === */
unsigned char B0_state, B1_state, B2_state;

/* === MODULE VARIABLES === */
bool init_module0_clock, init_module1_clock, init_module2_clock, init_module3_clock, init_module4_clock, init_module5_clock, init_module6_clock, init_module8_clock, init_module9_clock;

/* === MPU VARIABLES === */
int AcX, AcY, AcZ;
float temp;
bool mpuWake, danger;
char orientation;

/* === HEARTBEAT VARIABLES === */
unsigned long heartBeatOldTime;
int heartBeatTime, heartBeatState;

/* === 7-SEGMENT VARIABLES === */
byte displayOutput;

/* === TRAFFIC LIGHTS MANAGER VARIABLES === */
int trafficLightsStateEQ, trafficLightsState1P, trafficLightsState2P;
unsigned long trafficLightsTimeStampEQ, trafficLightsTimeStamp1P, trafficLightsTimeStamp2P;
bool b_trafficLightsEQ, b_trafficLights1P, b_trafficLights2P;

/* === LED VARIABLES === */
float brightness, fadeValue, fadeValueAmber, amberBrightness;
int i, j;

/* === FUNCTION PROTOTYPES === */
bool demandRequest();
void leaveHigh (unsigned char pin);
void pullLow (unsigned char pin);
void calculateXYZ();
void calculateTemp();
void mpuWakeUp();
void heartBeat();
void displayUpdate(byte eightBits);
byte numToBits(int number);
void trafficLightsEQ();
//void trafficLights1P();
//void trafficLights2P();
//void requestEvent();


void setup() {
  /* === BUTTON pinMode SETUP === */
  pinMode(_B0, INPUT); // _B0 is INPUT because it's conntected to D13, logics inverted
  pinMode(_B1, INPUT_PULLUP);
  pinMode(_B2, INPUT_PULLUP);

  // LEDs defined as OUTPUTs therefore NANO will expect an output at D2, D4 and not an INPUT
  pinMode(LED_RED_1, OUTPUT);
  pinMode(LED_YELLOW_1, OUTPUT);
  pinMode(LED_GREEN_1, OUTPUT);
  
  pinMode(LED_RED_2, OUTPUT);
  pinMode(LED_YELLOW_2, OUTPUT);
  pinMode(LED_GREEN_2, OUTPUT);

  pinMode(LED_TRI_R, OUTPUT);
  pinMode(LED_TRI_B, OUTPUT);
  pinMode(LED_TRI_G, OUTPUT);

  /* === SHIFT REGISTER pinMode SETUP === */
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  /* === CLOCK SETUP === */
  init_module0_clock = true; // BUTTON DEBOUNCER (SWITCH 0)
  init_module1_clock = true; // BUTTON DEBOUNCER (SWITCH 1)
  init_module2_clock = true; // BUTTON DEBOUNCER (SWITCH 2)
  init_module3_clock = true; // LED_RED_1 MANAGER
  init_module4_clock = true; // LED_GREEN_1 MANAGER
  init_module5_clock = true; // SCHEDULER
  init_module6_clock = true; // TRAFFIC LIGHTS DANGER INDICATOR
  init_module8_clock = true; // DISPLAY MANAGER
  init_module9_clock = true; // TRAFFIC LIGHTS MANAGER

  /* === BUTTON SETUP === */
  B0_state = notPRESSED;
  B1_state = notPRESSED;
  B2_state = notPRESSED;

  /* === RESOURCE MANAGER SETUP === */
  leaveHigh(GRANTED);
  managerState = NoTriggerNoDemand;
  trigger = false;

  /* === MPU SETUP === */
  mpuWake = true;

  /* === LED SETUP === */
  brightness = 0;
  fadeValue = 1;
  fadeValueAmber = 0.0588;
  amberBrightness = 0;
  i = 0;
  j = 0;

  /* === HEARTBEAT SETUP === */
  heartBeatTime = 500; // Setting the heartbeat delay to 500ms
  heartBeatState = 0; // Setting the heartbeat status to off

  /* === DISPLAY SETUP === */
  //displayOutput = B00111110;

  /* === TRAFFIC LIGHTS MANAGER SETUP === */
  trafficLightsStateEQ = 0;

  /* === SERIAL MONITOR SETUP === */
  Serial.begin(9600);

  /* === WIRE LIB SETUP === */
  Wire.begin(8);
  //Wire.onRequest(requestEvent());
}

void loop() {


  switch(managerState) {
    case NoTriggerNoDemand:
      //Serial.print("GRANTED: HIGH ");
      //Serial.println("NoTriggerNoDemand");
      leaveHigh(GRANTED);
      if(!demandRequest() && !trigger) {
      managerState = NoTriggerNoDemand;
      }
      else if(!demandRequest() && trigger) {
        managerState = TriggerIamMaster;
      }
      else if(demandRequest()) {
        managerState = IamSlave;
      }
      break;

    case TriggerIamMaster:
      //Serial.print("GRANTED: HIGH ");
      //Serial.println("TriggerIamMaster");
      leaveHigh(GRANTED);
      if(!trigger && !demandRequest()) {
        managerState = NoTriggerNoDemand;
      }
      else if(trigger) {
        managerState = TriggerIamMaster;
      }
      else if(!trigger && demandRequest()) {
        managerState = IamSlave;
      }
      break;

    case IamSlave:
      //Serial.print("GRANTED: LOW ");
      //Serial.println("IamSlave");
      pullLow(GRANTED);
      if(!demandRequest()) {
        managerState = NoTriggerNoDemand;
      }
      else if(demandRequest()) {
        managerState = IamSlave;
      }
      break;

    default:
      managerState = NoTriggerNoDemand;
      break;
    }

    /* === MODULE 0 - BUTTON DEBOUNCER (_B0) === */
  {
    static unsigned long module_time, module_delay, debounce_count;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module0_clock) {
      module_delay = 17;
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
          B0_state=notPRESSED;
          //digitalRead is inverted because it is wired to D13 (INVERTED LOGICS)
          if (!digitalRead(_B0)) state = 0;
          else {
            debounce_count = module_time;
            state = 1;
          }
          break;
        case 1: 
          B0_state=partialPRESS;
          if (!digitalRead(_B0)) state = 0;
          else if ((long)(millis() - debounce_count) < debounce) state = 1;
          else state = 2;
          break;
        case 2: 
          B0_state = normalPRESS;
          if (!digitalRead(_B0)) state = 0;
          else state = 2;
          break;
          
        default: 
          state = 0; 
          break;
      }
    }
  }

  /* === MODULE 1 - BUTTON DEBOUNCER (_B1) */
  {
    static unsigned long module_time, module_delay, debounce_count;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module1_clock) {
      module_delay = 17;
      module_time = millis();
      module_doStep = false;
      init_module1_clock = false;
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
          B1_state=notPRESSED;
          //digitalRead is inverted because it is wired to D13 (INVERTED LOGICS)
          if (digitalRead(_B1)) state = 0;
          else {
            debounce_count = module_time;
            state = 1;
          }
          break;
        case 1: 
          B1_state=partialPRESS;
          if (digitalRead(_B1)) state = 0;
          else if ((long)(millis() - debounce_count) < debounce) state = 1;
          else state = 2;
          break;
        case 2: 
          B1_state = normalPRESS;
          if (digitalRead(_B1)) state = 0;
          else state = 2;
          break;
          
        default: 
          state = 0; 
          break;
      }
    }
  }

  /* === MODULE 2 - BUTTON DEBOUNCER (_B2) === */
  {
    static unsigned long module_time, module_delay, debounce_count;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module2_clock) {
      module_delay = 17;
      module_time = millis();
      module_doStep = false;
      init_module2_clock = false;
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
          B2_state=notPRESSED;
          if (digitalRead(_B2)) state = 0;
          else {
            debounce_count = module_time;
            state = 1;
          }
          break;
        case 1: 
          B2_state=partialPRESS;
          if (digitalRead(_B2)) state = 0;
          else if ((long)(millis() - debounce_count) < debounce) state = 1;
          else state = 2;
          break;
        case 2: 
          B2_state = normalPRESS;
          if (digitalRead(_B2)) state = 0;
          else state = 2;
          break;
          
        default: 
          state = 0; 
          break;
      }
    }
  }

  /* === MODULE 3 - 3-COLOR MODE MANAGER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state;
    static bool firstTime;
    
    if (init_module3_clock) {
      module_delay = 4;
      module_time = millis();
      module_doStep = false;
      init_module3_clock = false;
      state = 0;
      firstTime = true;
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
        case 0: // OFF
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_G, LOW);
          digitalWrite(LED_TRI_B, LOW);
          if(B0_state == normalPRESS) {
            state = 1;
            firstTime = true;
          }
          break;

        case 1: // OFF
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_G, LOW);
          digitalWrite(LED_TRI_B, LOW);
          if(B0_state == notPRESSED) {
            state = 2;
          }
          break;
        case 2: // AMBER (WORKS DECENT ENOUGH BUT NEEDS SOME TWEAKING) // SOMEWHAT DONE

          switch(i) {
            case 0:
              analogWrite(LED_TRI_R, brightness);
              analogWrite(LED_TRI_B, 0);
              brightness = brightness + fadeValue;

              if(brightness <= 0 || brightness >= 255) {
                i = 1;
              }
            break;

            case 1:
              fadeValue = -fadeValue;
              i = 0;
            break;
          }

          switch(j) {
            case 0:
              analogWrite(LED_TRI_G, amberBrightness);
              analogWrite(LED_TRI_B, 0);
              amberBrightness = amberBrightness + fadeValueAmber;

              if(amberBrightness <= 0 || amberBrightness >= 15) {
                j = 1;
              }
            break;

            case 1:
              fadeValueAmber = -fadeValueAmber;
              j = 0;
            break;
          }

          if(B0_state == normalPRESS) {
            state = 3;
          }

        break;
        

        case 3: // AMBER (WORKS DECENT ENOUGH BUT NEEDS SOME TWEAKING)
          switch(i) {
            case 0:
              analogWrite(LED_TRI_R, brightness);
              analogWrite(LED_TRI_B, 0);
              brightness = brightness + fadeValue;

              if(brightness <= 0 || brightness >= 255) {
                i = 1;
              }
            break;

            case 1:
              fadeValue = -fadeValue;
              i = 0;
            break;
          }

          switch(j) {
            case 0:
              analogWrite(LED_TRI_G, amberBrightness);
              analogWrite(LED_TRI_B, 0);
              amberBrightness = amberBrightness + fadeValueAmber;

              if(amberBrightness <= 0 || amberBrightness >= 15) {
                j = 1;
              }
            break;

            case 1:
              fadeValueAmber = -fadeValueAmber;
              j = 0;
            break;
          }

          if(B0_state == notPRESSED) {
            state = 4;
          }
        break;
        case 4: // BLUE
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_G, LOW);
          analogWrite(LED_TRI_B, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == normalPRESS) {
            state = 5;
          }
          break;

        case 5: // BLUE
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_G, LOW);
          analogWrite(LED_TRI_B, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == notPRESSED) {
            state = 6;
          }
          break;
        case 6: // GREEN
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_B, LOW);
          analogWrite(LED_TRI_G, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == normalPRESS) {
            state = 7;
          }
          break;

        case 7: // GREEN
          digitalWrite(LED_TRI_R, LOW);
          digitalWrite(LED_TRI_B, LOW);
          analogWrite(LED_TRI_G, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == notPRESSED) {
            state = 8;
          }
          break;
        case 8: // RED
          digitalWrite(LED_TRI_G, LOW);
          digitalWrite(LED_TRI_B, LOW);
          analogWrite(LED_TRI_R, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == normalPRESS) {
            state = 9;
          }
          break;

        case 9: // RED
          digitalWrite(LED_TRI_G, LOW);
          digitalWrite(LED_TRI_B, LOW);
          analogWrite(LED_TRI_R, brightness);
          brightness = brightness + fadeValue;
          amberBrightness = amberBrightness + fadeValueAmber;

          if(brightness <= 0 || brightness >= 255) {
            fadeValue = -fadeValue;
          }
          if(amberBrightness <= 0 || amberBrightness >= 15) {
            fadeValueAmber = -fadeValueAmber;
          }

          if(B0_state == notPRESSED) {
            state = 0;
          }
          break;

        default: state = 0;
      }
    }
  }

  /* === MODULE 4 - FRUIT MACHINE === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state0, state1;
    static bool firstTime;
    
    if (init_module4_clock) {
      module_delay = 50;
      module_time = millis();
      module_doStep = false;
      init_module4_clock = false;
      state0 = 0;
      state1 = 0;
      firstTime = true;
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
      if(firstTime) {
        digitalWrite(LED_TRI_R, LOW);
        digitalWrite(LED_TRI_G, LOW);
        digitalWrite(LED_TRI_B, LOW);
        firstTime = false;
      }
      if(B0_state == normalPRESS) {
        switch(state0) {
          case 0: 
            digitalWrite(LED_YELLOW_1, LOW);
            digitalWrite(LED_GREEN_1, LOW);
            digitalWrite(LED_RED_1, HIGH);
            state0 = 1;
            break;
          case 1: 
            digitalWrite(LED_YELLOW_1, HIGH);
            digitalWrite(LED_GREEN_1, LOW);
            digitalWrite(LED_RED_1, LOW);
            state0 = 2;
            break;
          case 2:
            digitalWrite(LED_YELLOW_1, LOW);
            digitalWrite(LED_GREEN_1, HIGH);
            digitalWrite(LED_RED_1, LOW);
            state0 = 0;
            break;
          default: 
            state0 = 0; 
            break;
        }  
      }

      else {
        digitalWrite(LED_YELLOW_1, LOW);
        digitalWrite(LED_GREEN_1, LOW);
        digitalWrite(LED_RED_1, LOW);
        state0 = 0;
      }

      if (B2_state == normalPRESS) {
        switch(state1) {
          case 0: 
            digitalWrite(LED_YELLOW_2, LOW);
            digitalWrite(LED_GREEN_2, LOW);
            digitalWrite(LED_RED_2, HIGH);
            state1 = 1;
            break;
          case 1: 
            digitalWrite(LED_YELLOW_2, HIGH);
            digitalWrite(LED_GREEN_2, LOW);
            digitalWrite(LED_RED_2, LOW);
            state1 = 2;
            break;
          case 2:
            digitalWrite(LED_YELLOW_2, LOW);
            digitalWrite(LED_GREEN_2, HIGH);
            digitalWrite(LED_RED_2, LOW);
            state1 = 0;
            break;
          default: 
            state1 = 0; 
            break;
        }
      }

      else {
        digitalWrite(LED_YELLOW_2, LOW);
        digitalWrite(LED_GREEN_2, LOW);
        digitalWrite(LED_RED_2, LOW);
        state1 = 0;
      }
    }
  }

  /* === MODULE 5 - SCHEDULER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module5_clock) {
      module_delay = 1;
      module_time = millis();
      module_doStep = false;
      init_module5_clock = false;
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
        case 0:   // MODULE 3 (3-COLOR-MODE) OFF, MODULE 4 (FRUIT MACHINE) OFF, MODULE 9 OFF, WAITING FOR THE BUTTON TO BE PRESSED
          init_module3_clock = true; // 3-COLOR
          init_module4_clock = true; // FRUIT MACHINE
          init_module9_clock = true; // TRAFFIC LIGHTS EQ
          if (B1_state == normalPRESS)
          {
            state = 1;
          }
          break;
          
        case 1:   // MODULE 3 (3-COLOR-MODE) OFF, MODULE 4 (FRUIT MACHINE) OFF, MODULE 9 OFF WAITING FOR THE BUTTON TO BE RELEASED
          init_module3_clock = true;            
          init_module4_clock = true;
          init_module9_clock = true; // TRAFFIC LIGHTS EQ
          if (B1_state == notPRESSED) {
            state = 2;           
          }
          break;

        case 2:   // MODULE 3 ON, MODULE 4 OFF, MODULE 9 ON, WAITING FOR THE BUTTON TO BE PRESSED
          init_module3_clock = false;
          init_module4_clock = true;
          init_module9_clock = false; // TRAFFIC LIGHTS EQ
          if (B1_state == normalPRESS) {
            state = 3;           
          }
          break;
            
        case 3:   // MODULE 3 ON, MODULE 4 OFF, MODULE 9 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module3_clock = false;
          init_module4_clock = true;
          init_module9_clock = false; // TRAFFIC LIGHTS EQ
          if (B1_state == notPRESSED) {
            state = 4;           
          }
          break;

        case 4:   // MODULE 3 OFF, MODULE 4 ON, MODULE 9 OFF, WAITING FOR THE BUTTON TO BE PRESSED
          init_module3_clock = true;
          init_module4_clock = false;
          init_module9_clock = true; // TRAFFIC LIGHTS EQ
          if (B1_state == normalPRESS) {
            state = 5;           
          }
          break;
            
        case 5:   // MODULE 3 OFF, MODULE 4 ON, MODULE 9 OFF, WAITING FOR THE BUTTON TO BE RELEASED
          init_module3_clock = true;
          init_module4_clock = false;
          init_module9_clock = true; // TRAFFIC LIGHTS EQ
          if (B1_state == notPRESSED) {
            state = 0; // STATE 0, MUST BE CHANGED TO 6 FOR OTHER MODULES TO WORK !!!  
          }
          break;

        /*case 6:   // MODULE 9 ON, MODULE 4 ON, WAITING FOR THE BUTTON TO BE PRESSED
          init_module3_clock = false;
          init_module4_clock = false;
          if (B1_state == normalPRESS) {
            state = 7;           
          }
          break;
            
        case 7:   // MODULE 3 ON, MODULE 4 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module3_clock = false;
          init_module4_clock = false;
          if (B1_state == notPRESSED) {
            state = 0;           
          }
          break;*/

        default: 
          state = 0; 
          break;
      }  
    }
  }

  /* === MODULE 6 - TRAFFIC LIGHTS DANGER INDICATOR === */

  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    unsigned long timeStamp;
    
    if (init_module6_clock) {
      module_delay = 10;
      module_time = millis();
      module_doStep = false;
      init_module6_clock = false;
      state=0;
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
          timeStamp = millis();
        case 1:
          //Serial.println("SWITCH I: CASE 1");
          if(((long)(millis() - timeStamp)) < 15) {
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
          if(mpuWake) {
            mpuWakeUp();
          }

          trigger = false;
          calculateXYZ();
          calculateTemp();
          //Serial.print("arduino: ");
          //Serial.print(temp);
          //Serial.print(", ");
          //Serial.println(danger);
          state = 0;
          break;

          default: 
          state = 0;
          break;
      }  
    }
  }

  /* === MODULE 7 - HEARTBEAT MANAGER === */
  {
    static unsigned long module_time = millis(), module_delay = 500;
    
      if (((unsigned long)(millis() - module_time)) > module_delay) {
        module_time += module_delay;
        heartBeat();
      }
  }

  /* === MODULE 8 - DISPLAY MANAGER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    
    if (init_module8_clock) {
      module_delay = 1;
      module_time = millis();
      module_doStep = false;
      init_module8_clock = false;
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
      displayUpdate(displayOutput);
    }
  }

  /* === MODULE 9 - TRAFFIC LIGHTS MANAGER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static int state;
    //unsigned long timeStamp;
    
    if (init_module9_clock) {
      module_delay = 5;
      module_time = millis();
      module_doStep = false;
      state = 0;
      trafficLightsStateEQ = 0;
      trafficLightsTimeStampEQ = millis();
      init_module9_clock = false;
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
      // THE DISPLAY DOESN'T WORK PROPERLY BUT IT'S FOR DEBUG PURPOSES SO IT'S FINE, SEQUENCES WORK FINE BUT STILL EVERY ALREADY RUNNING SEQUENCE MUST FINISH BEFORE A NEW ONE STARTS
      switch(state) {
        case 0: // EQ
          displayOutput |= B11101110;
          //displayUpdate(displayOutput);

          b_trafficLightsEQ = true; // EQUAL MODE
          b_trafficLights1P = false; // 1 SET PRIORITY MODE
          b_trafficLights2P = false; // 2 SET PRIORITY MODE
          if(B2_state == normalPRESS) {
            state = 1;
          }
          break;

        case 1: // EQ
          displayOutput |= B11101110;
          //displayUpdate(displayOutput);

          b_trafficLightsEQ = true; // EQUAL MODE
          b_trafficLights1P = false; // 1 SET PRIORITY MODE
          b_trafficLights2P = false; // 2 SET PRIORITY MODE
          if(B2_state == notPRESSED) {
            state = 2;
          }
          break;

        case 2: // SET 1 PRIORITY
          displayOutput |= B00111110;
          //displayUpdate(displayOutput);

          b_trafficLightsEQ = false; // EQUAL MODE
          b_trafficLights1P = true; // 1 SET PRIORITY MODE
          b_trafficLights2P = false; // 2 SET PRIORITY MODE
          if(B2_state == normalPRESS) {
            state = 3;
          }
          break;

        case 3: // SET 1 PRIORITY
          displayOutput |= B00111110;
          //displayUpdate(displayOutput);

          b_trafficLightsEQ = false; // EQUAL MODE
          b_trafficLights1P = true; // 1 SET PRIORITY MODE
          b_trafficLights2P = false; // 2 SET PRIORITY MODE
          if(B2_state == notPRESSED) {
            state = 4;
          }
          break;

        case 4: // SET 2 PRIORITY
          displayOutput |= B10011100;
          //displayUpdate(displayOutput);
          
          b_trafficLightsEQ = false; // EQUAL MODE
          b_trafficLights1P = false; // 1 SET PRIORITY MODE
          b_trafficLights2P = true; // 2 SET PRIORITY MODE
          if(B2_state == normalPRESS) {
            state = 5;
          }
          break;

        case 5: // SET 2 PRIORITY
          displayOutput |= B10011100;

          b_trafficLightsEQ = false; // EQUAL MODE
          b_trafficLights1P = false; // 1 SET PRIORITY MODE
          b_trafficLights2P = true; // 2 SET PRIORITY MODE
          if(B2_state == notPRESSED) {
            state = 0;
          }
          break;
      }

      displayUpdate(displayOutput);
      trafficLightsEQ();
    }
  }

}



/* === FUNCTIONS === */

bool demandRequest() {
  return (!(analogRead(ANALOG_INPUT) >> 8));
}

void leaveHigh (unsigned char pin) {
  pinMode(pin,INPUT);
}

void pullLow (unsigned char pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

void calculateXYZ() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(ACCEL_XOUT_H); // starting with register ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6 ,true);
      
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  Wire.endTransmission(true);

  if (abs(AcZ)>1.75*abs(AcX) && abs(AcZ)>1.75*abs(AcY)) {
    if (AcZ < 0) {
      //Serial.println("FACE DOWN (1)"); // FACE DOWN
      orientation = 'b'; // base up (b)
      danger = true;
    }
    else {
      //Serial.println("FACE UP (2)"); // FACE UP
      orientation = 'F'; //FLAT (F)
      danger = false;
      }
    }

  if (abs(AcX)>1.75*abs(AcZ) && abs(AcX)>1.75*abs(AcY)) {
    if (AcX < 0){
      //Serial.println("FACING OPPOSITE OF ME (3)"); // FACING OPPOSITE OF ME
      orientation = 'L'; //landscape (L)
      danger = true;
    }
    else {
       //Serial.println("FACING ME (4)"); // FACING ME
      orientation = 'U'; //landscape upside down (U)
      danger = true;
    }
  }
  if (abs(AcY)>1.75*abs(AcX) && abs(AcY)>1.75*abs(AcZ)) {
    if (AcY < 0) {
      //Serial.println("TO THE LEFT (5)"); // TO THE LEFT
      orientation = 'l'; //portrait left (l)
      danger = true;
    }
    else {
      //Serial.println("TO THE RIGHT (6)"); // TO THE RIGHT
      orientation = 'r'; //portrait right (r)
      danger = true;
    }
  }
}

void calculateTemp() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(TEMP_OUT_H); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 2 ,true);
      
  temp = Wire.read() << 8 | Wire.read();

  temp = temp / 340.00 + 36.53;

  Wire.endTransmission(true);
}

void mpuWakeUp() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  mpuWake = false;
}

// I am totally unsure if this is correct or not but it works...
void heartBeat() {
  switch(heartBeatState) {
    case 0:
      displayOutput |= B00000001;
      displayUpdate(displayOutput);
      heartBeatState = 1;
      break;
    case 1:
      displayOutput &= B00000000;
      displayUpdate(displayOutput);
      heartBeatState = 0;
      break;
  }
}

void trafficLightsEQ() {

  switch(trafficLightsStateEQ) {
    case 0: // STARTUP PHASE
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 2000) {
        trafficLightsStateEQ = 0;
        digitalWrite(LED_YELLOW_1, HIGH);
        digitalWrite(LED_YELLOW_2, HIGH);
      }
      else {
        digitalWrite(LED_YELLOW_1, LOW);
        digitalWrite(LED_YELLOW_2, LOW);
        trafficLightsTimeStampEQ = millis();
        trafficLightsStateEQ = 1;
      }
      break;

    case 1: // STATE 1 (1 - R; 2 - R)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        Serial.println(trafficLightsTimeStampEQ);
        trafficLightsStateEQ = 1;
        digitalWrite(LED_RED_1, HIGH);
        digitalWrite(LED_RED_2, HIGH);
      }
      else {
        trafficLightsStateEQ = 2;
        trafficLightsTimeStampEQ = millis();
      }
      break;

    case 2: // STATE 2 (1 - RY; 2 - R)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        trafficLightsStateEQ = 2;
        digitalWrite(LED_RED_1, HIGH);
        digitalWrite(LED_YELLOW_1, HIGH);

        digitalWrite(LED_RED_2, HIGH);
      }
      else {
        trafficLightsStateEQ = 3;
        digitalWrite(LED_RED_1, LOW);
        digitalWrite(LED_YELLOW_1, LOW);
        trafficLightsTimeStampEQ = millis();
      }
      break;
    
    case 3: // STATE 3 (1 - G; 2 - R)
      // IF TRAFFIC LIGHTS EQUAL
      if(b_trafficLightsEQ) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 4000) {
          trafficLightsStateEQ = 3;
          digitalWrite(LED_GREEN_1, HIGH);

          digitalWrite(LED_RED_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_1, LOW);
          trafficLightsStateEQ = 4;
          trafficLightsTimeStampEQ = millis();
        }
      }
      
      // IF TRAFFIC LIGHTS IN SET 1 PRIORITY MODE
      else if(b_trafficLights1P) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 6000) {
          trafficLightsStateEQ = 3;
          digitalWrite(LED_GREEN_1, HIGH);

          digitalWrite(LED_RED_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_1, LOW);
          trafficLightsStateEQ = 4;
          trafficLightsTimeStampEQ = millis();
        }
      }

      else if(b_trafficLights2P) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 2000) {
          trafficLightsStateEQ = 3;
          digitalWrite(LED_GREEN_1, HIGH);

          digitalWrite(LED_RED_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_1, LOW);
          trafficLightsStateEQ = 4;
          trafficLightsTimeStampEQ = millis();
        }
      }


      break;

    case 4: // STATE 4 (1 - Y; 2 - R)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        trafficLightsStateEQ = 4;
        digitalWrite(LED_YELLOW_1, HIGH);

        digitalWrite(LED_RED_2, HIGH);
      }
      else {
        digitalWrite(LED_YELLOW_1, LOW);
        trafficLightsStateEQ = 5;
        trafficLightsTimeStampEQ = millis();
      }
      break;

    case 5: // STATE 5 (1 - R, 2 - R)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        trafficLightsStateEQ = 5;
        digitalWrite(LED_RED_1, HIGH);

        digitalWrite(LED_RED_2, HIGH);
      }
      else {
        trafficLightsStateEQ = 6;
        trafficLightsTimeStampEQ = millis();
      }
      break;

    case 6: // STATE 6 (1 - R; 2 - RY)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        trafficLightsStateEQ = 6;
        digitalWrite(LED_RED_1, HIGH);

        digitalWrite(LED_RED_2, HIGH);
        digitalWrite(LED_YELLOW_2, HIGH);
      }
      else {
        digitalWrite(LED_RED_2, LOW);
        digitalWrite(LED_YELLOW_2, LOW);
        trafficLightsStateEQ = 7;
        trafficLightsTimeStampEQ = millis();
      }
      break;

    case 7: // STATE 7 (1 - R; 2 - G)

      // IF EQUAL PRIORITY
      if(b_trafficLightsEQ) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 4000) {
          trafficLightsStateEQ = 7;
          digitalWrite(LED_RED_1, HIGH);

          digitalWrite(LED_GREEN_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_2, LOW);
          trafficLightsStateEQ = 8;
          trafficLightsTimeStampEQ = millis();
        }
      }
      // IF SET 1 PRIORITY
      else if(b_trafficLights1P) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 2000) {
          trafficLightsStateEQ = 7;
          digitalWrite(LED_RED_1, HIGH);

          digitalWrite(LED_GREEN_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_2, LOW);
          trafficLightsStateEQ = 8;
          trafficLightsTimeStampEQ = millis();
        }
      }
      // IF SET 2 PRIORITY
      else if(b_trafficLights2P) {
        if(((long)(millis() - trafficLightsTimeStampEQ)) < 6000) {
          trafficLightsStateEQ = 7;
          digitalWrite(LED_RED_1, HIGH);

          digitalWrite(LED_GREEN_2, HIGH);
        }
        else {
          digitalWrite(LED_GREEN_2, LOW);
          trafficLightsStateEQ = 8;
          trafficLightsTimeStampEQ = millis();
        }
      }
  
      break;
    
    case 8: // STATE 8 (1 - R; 2 - Y)
      if(((long)(millis() - trafficLightsTimeStampEQ)) < 1000) {
        trafficLightsStateEQ = 8;
        digitalWrite(LED_RED_1, HIGH);

        digitalWrite(LED_YELLOW_2, HIGH);
      }
      else {
        digitalWrite(LED_YELLOW_2, LOW);
        trafficLightsStateEQ = 1;
        trafficLightsTimeStampEQ = millis();
      }
      break;
  }
}

void displayUpdate(byte eightBits) {
  digitalWrite(latchPin, LOW); // Prepare the shift register for data
  shiftOut(dataPin, clockPin, LSBFIRST, eightBits); // Send the data
  digitalWrite(latchPin, HIGH); // Update the display
}

byte numToBits(int number) {
  switch (number) {
    case 0:
      return B11101110; //A
      break;
    case 1:
      return B00111110; //b
      break;
    case 2:
      return B10011100; //C
      break;
    case 3:
      return B01001111; //3
      break;
    case 4:
      return B01100110; //4
      break;
    case 5:
      return B01101101; //5
      break;
    case 6:
      return B01111101; //6
      break;
    case 7:
      return B00000111; //7
      break;
    case 8:
      return B01111111; //8
      break;
    case 9:
      return B01101111; //9
      break;
    case 10:
      return B01110111; //A
      break;
    case 11:
      return B01111100; //b
      break;
    case 12:
      return B00111001; //C
      break;
    case 13:
      return B01011110; //d
      break;
    case 14:
      return B01111001; //E
      break;
    case 15:
      return B01110001; //F
      break;
    case 16:
      return B00000001; //.
      break;
    default:
      return B10010010; // Error condition, displays three vertical bars
      break;
  }
}
