#include <Arduino.h>
#include <Wire.h>

/* === UI MACROS === */
#define _B1 13

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
#define LED_GREEN_1 4 // GREEN 1st row

/* === BUTTON STATES MACROS === */
#define notPRESSED 0
#define partialPRESS 1
#define normalPRESS 2
#define debounce 500 // DEBOUNCE DURATION

/* === RESOURCE MANAGER VARIABLES === */
int managerState;
bool trigger;

/* === BUTTON VARIABLES === */
unsigned char B1_state;

/* === MODULE VARIABLES === */
bool init_module0_clock, init_module1_clock, init_module2_clock, init_module3_clock, init_module4_clock;

/* === MPU VARIABLES === */
int AcX, AcY, AcZ;
float temp;
bool mpuWake, danger;
char orientation;

/* === FUNCTION PROTOTYPES === */
bool demandRequest();
void leaveHigh (unsigned char pin);
void pullLow (unsigned char pin);
void calculateXYZ();
void calculateTemp();
void mpuWakeUp();
//void requestEvent();


void setup() {
  pinMode(_B1, INPUT); //_B1 is INPUT because it's conntected to D13, logics inverted

  //LEDs defined as OUTPUTs therefore NANO will expect an output at D2, D4 and not an INPUT
  pinMode(LED_RED_1, OUTPUT);
  pinMode(LED_GREEN_1, OUTPUT);

  //Setting all clocks to true
  init_module0_clock = true;
  init_module1_clock = true;
  init_module2_clock = true;
  init_module3_clock = true;
  init_module4_clock = true;

  //_B1 is not pressed by default
  B1_state = notPRESSED;

  /* === RESOURCE MANAGER SETUP === */
  leaveHigh(GRANTED);
  managerState = NoTriggerNoDemand;
  trigger = false;

  /* === MPU SETUP === */
  mpuWake = true;

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

    /* === MODULE 0 - BUTTON DEBOUNCER === */
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
          B1_state=notPRESSED;
          //digitalRead is inverted because it is wired to D13 (INVERTED LOGICS)
          if (!digitalRead(_B1)) state = 0;
          else {
            debounce_count = module_time;
            state = 1;
          }
          break;
        case 1: 
          B1_state=partialPRESS;
          if (!digitalRead(_B1)) state = 0;
          else if ((long)(millis() - debounce_count) < debounce) state = 1;
          else state = 2;
          break;
        case 2: 
          B1_state = normalPRESS;
          if (!digitalRead(_B1)) state = 0;
          else state = 2;
          break;
          
        default: 
          state = 0; 
          break;
      }
    }
  }

  /* === MODULE 1 - LED_RED_1 MANAGER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module1_clock) {
      module_delay = 100;
      module_time = millis();
      module_doStep = false;
      init_module1_clock = false;
      state=0;
      digitalWrite(LED_RED_1, LOW);
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
          digitalWrite(LED_RED_1, HIGH);
            state = 1;
            break;
        case 1: 
          digitalWrite(LED_RED_1, LOW);
          state = 0;
          break;
        default: 
          state = 0; 
          break;
      }  
    }
  }

  /* === MODULE 2 - LED_GREEN_1 MANAGER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module2_clock) {
      module_delay = 270;
      module_time = millis();
      module_doStep = false;
      init_module2_clock = false;
      state=0;
      digitalWrite(LED_GREEN_1, LOW);
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
          digitalWrite(LED_GREEN_1, HIGH);
            state = 1;
            break;
        case 1: 
          digitalWrite(LED_GREEN_1, LOW);
          state = 0;
          break;
        default: 
          state = 0; 
          break;
      }  
    }
  }

  /* === MODULE 3 - SCHEDULER === */
  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    
    if (init_module3_clock) {
      module_delay = 10;
      module_time = millis();
      module_doStep = false;
      init_module3_clock = false;
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
        case 0:   // MODULE 1 OFF, MODULE 2 OFF, WAITING FOR THE BUTTON TO BE PRESSED
          init_module1_clock = true;
          init_module2_clock = true;
          if (B1_state == normalPRESS)
          {
            state = 1;           
          }
          break;
          
        case 1:   // MODULE 1 OFF, MODULE 2 OFF, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = true;            
          init_module2_clock = true;
          if (B1_state == notPRESSED) {
            state = 2;           
          }
          break;

        case 2:   // MODULE 1 ON, MODULE 2 OFF, WAITING FOR THE BUTTON TO BE PRESSED
          init_module1_clock = false;
          init_module2_clock = true;
          if (B1_state == normalPRESS) {
            state = 3;           
          }
          break;
            
        case 3:   // MODULE 1 ON, MODULE 2 OFF, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = false;
          init_module2_clock = true;
          if (B1_state == notPRESSED) {
            state = 4;           
          }
          break;

        case 4:   // MODULE 1 OFF, MODULE 2 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = true;
          init_module2_clock = false;
          if (B1_state == normalPRESS) {
            state = 5;           
          }
          break;
            
        case 5:   // MODULE 1 OFF, MODULE 2 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = true;
          init_module2_clock = false;
          if (B1_state == notPRESSED) {
            state = 6;           
          }
          break;

        case 6:   // MODULE 1 ON, MODULE 2 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = false;
          init_module2_clock = false;
          if (B1_state == normalPRESS) {
            state = 7;           
          }
          break;
            
        case 7:   // MODULE 1 ON, MODULE 2 ON, WAITING FOR THE BUTTON TO BE RELEASED
          init_module1_clock = false;
          init_module2_clock = false;
          if (B1_state == notPRESSED) {
            state = 0;           
          }
          break;

        default: 
          state = 0; 
          break;
      }  
    }
  }

  /* === MODULE 4 - TRAFFIC LIGHTS DANGER INDICATOR === */

  {
    static unsigned long module_time, module_delay;
    static bool module_doStep;
    static unsigned char state; // state variable for module 0
    unsigned long timeStamp;
    
    if (init_module4_clock) {
      module_delay = 10;
      module_time = millis();
      module_doStep = false;
      init_module4_clock = false;
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
          Serial.print("arduino: ");
          Serial.print(temp);
          Serial.print(", ");
          Serial.println(danger);
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