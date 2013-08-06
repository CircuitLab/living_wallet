#include <MsTimer2.h>

// Macros for Pins of Da Vinci 32u4
#define PB0 17 // BIN_REAR_1
#define PB1 15 // BIN_REAR_2
#define PB2 16 // STBY_FRONT
#define PB3 14 // BIN_FRONT_1
#define PB4 8  // AIN_FRONT_1
#define PB5 9  // AIN_FRONT_2
#define PB6 10 // PWM_REAR_B
#define PB7 11 // PWM_FRONT_A
#define PC6 5  // PWM_FRONT_B
#define PC7 13 // KONASHI_PIN_4
#define PD0 3
#define PD1 2  // AIN_REAR_2
#define PD2 0  // RX
#define PD3 1  // TX
#define PD4 4  // AIN_REAR_1
#define PD5    // (TXLED)
#define PD6 12 // BIN_FRONT_2
#define PD7 6  // PWM_REAR_A
#define PE2    // (HWB)
#define PE6 7  // KONASHI_PIN_5
#define PF0 23 // (A5) FLEX_SENSOR_PIN
#define PF1 22 // (A4)
#define PF4 21 // (A3) IR_SENSOR_PIN
#define PF5 20 // (A2)
#define PF6 19 // (A1) STBY_REAR
#define PF7 18 // (A0) KONASHI_PIN_3


#define STATE_NEUTRAL 0
#define STATE_ENABLE_CONSUME 1
#define STATE_DISABLE_CONSUME 2

// MsTimer2でD3とD11を使用

// Front Motor Stand-by
#define FRONT_STBY PF6

// Front Motor A
#define FRONT_PWM_A PD7 //OK
#define FRONT_AIN_1 PD4 //OK
#define FRONT_AIN_2 PD1 //OK

// Front Motor B
#define FRONT_PWM_B PB6 //OK
#define FRONT_BIN_1 PB0 //OK
#define FRONT_BIN_2 PB1 //OK


// Rear Motor Stand-by
#define REAR_STBY PB2

// Rear Motor A
#define REAR_PWM_A PB7 //Speed control 
#define REAR_AIN_1 PB4 //Direction
#define REAR_AIN_2 PB5 //Direction

// Rear Motor B
#define REAR_PWM_B PC6 //Speed control
#define REAR_BIN_1 PB3 //Direction
#define REAR_BIN_2 PD6 //Direction

#define IR_SENSOR_PIN PF4
#define IR_SENSOR_THRESHOLD 600
//#define IR_LED_PIN 13
volatile int irSensorValue = 0;

#define FLEX_SENSOR_PIN PF0
#define FLEX_SENSOR_THRESHOLD 700
volatile int flexSensorValue = 0;

//#define STATE_PIN PF4
//volatile int state = LOW;

#define KONASHI_PIN_3 PF7
#define KONASHI_PIN_4 PC7
#define KONASHI_PIN_5 PE6
//#define KONASHI_PIN_5_INTERRUPT 4
volatile int konashiPin4PrevValue = LOW;
volatile int konashiPin5PrevValue = LOW;
volatile int konashiPin4Value = LOW;
volatile int konashiPin5Value = LOW;

#define FRONT_WHEEL_RIGHT 0
#define FRONT_WHEEL_LEFT 1
#define REAR_WHEEL_RIGHT 0
#define REAR_WHEEL_LEFT 1
#define DIRECTION_FORWARD 1
#define DIRECTION_BACK 0


volatile int prevState;
volatile int currentState;

boolean bAvoided = false;
boolean enableAvoidance = false;


// Arduinoとのコミュニケーション用
#define START_BYTE 0x7E
#define END_BYTE 0x88
#define COMMAND_NEUTRAL 0x00
#define COMMAND_ENABLE_CONSUME 0x10
#define COMMAND_DISABLE_CONSUME 0x20
#define COMMAND_CAUGHT 0x30

void setup()
{
  Serial.begin(9600);
  Serial1.begin(57600);
  
  analogReference(INTERNAL);
  
  // setup front motors
  pinMode(FRONT_STBY, OUTPUT);

  pinMode(FRONT_PWM_A, OUTPUT);
  pinMode(FRONT_AIN_1, OUTPUT);
  pinMode(FRONT_AIN_2, OUTPUT);

  pinMode(FRONT_PWM_B, OUTPUT);
  pinMode(FRONT_BIN_1, OUTPUT);
  pinMode(FRONT_BIN_2, OUTPUT);
  
  // setup rear motors
  pinMode(REAR_STBY, OUTPUT);
  
  pinMode(REAR_PWM_A, OUTPUT);
  pinMode(REAR_AIN_1, OUTPUT);
  pinMode(REAR_AIN_2, OUTPUT);
  
  pinMode(REAR_PWM_B, OUTPUT);
  pinMode(REAR_BIN_1, OUTPUT);
  pinMode(REAR_BIN_2, OUTPUT);
  
  pinMode(KONASHI_PIN_3, OUTPUT);
  pinMode(KONASHI_PIN_5, INPUT);
  
  digitalWrite(KONASHI_PIN_3, LOW);
  konashiPin4PrevValue = digitalRead(KONASHI_PIN_4);
  konashiPin5PrevValue = digitalRead(KONASHI_PIN_5);
  
  // KONASHIの4ピンがLOWなら消費可能、HIGHなら不可能
  if (konashiPin4PrevValue == konashiPin5PrevValue) {
    currentState = STATE_NEUTRAL;
  } else if (HIGH == konashiPin4PrevValue) {
    currentState = STATE_ENABLE_CONSUME;
  } else if (HIGH == konashiPin5PrevValue) {
    currentState = STATE_DISABLE_CONSUME;
  } 
  
  prevState = currentState;
  
//  attachInterrupt(KONASHI_PIN_5_INTERRUPT, changeStateByKonashi, CHANGE);
  
  MsTimer2::set(100, observeInputs);
  MsTimer2::start();
}

void loop()
{
  String serialMsg = "";
  while (Serial1.available()) {
    serialMsg += Serial1.read();
//    Serial.println(Serial1.read());
  }
  
  if (0 != strcmp(serialMsg.c_str(), "")) {
    Serial.print("serial input: ");
    Serial.println(serialMsg);
  }
  
  if (STATE_NEUTRAL == currentState) {
   moveFront(FRONT_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
    delay(10);
    moveFront(FRONT_WHEEL_LEFT, 255, DIRECTION_FORWARD);
    delay(500);
    stopFront();
//    delay(100);
    
    moveRear(REAR_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
    delay(10);;
    moveRear(REAR_WHEEL_LEFT, 255, DIRECTION_FORWARD);
    delay(500);
    stopRear();
  } else if (STATE_ENABLE_CONSUME == currentState) {
    moveFront(FRONT_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
    delay(10);
    moveFront(FRONT_WHEEL_LEFT, 255, DIRECTION_FORWARD);
    delay(500);
    stopFront();
//    delay(100);
    
    moveRear(REAR_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
    delay(10);;
    moveRear(REAR_WHEEL_LEFT, 255, DIRECTION_FORWARD);
    delay(500);
    stopRear();
//    delay(100);
  } else if (STATE_DISABLE_CONSUME == currentState) {
    if (enableAvoidance) {
      moveFront(FRONT_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
      delay(10);
      moveFront(FRONT_WHEEL_LEFT, 100, DIRECTION_BACK);
      delay(10);
      stopFront();
      
      moveRear(REAR_WHEEL_RIGHT, 255, DIRECTION_FORWARD);
      delay(10);
      moveRear(REAR_WHEEL_LEFT, 100, DIRECTION_BACK);
      delay(10);
      stopRear();
    } else {
      moveFront(0, 255, 1);
      moveFront(1, 255, 1);
      delay(200);
      stopFront();
      
      moveRear(0, 255, 0);
      moveRear(1, 255, 0);
      delay(200);
      stopRear();
    }
  }
}

// 前輪を動かす
void moveFront(int motor, int speed, int direction)
{ 
  digitalWrite(FRONT_STBY, HIGH); //disable standby

  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if (1 == direction) {
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if (0 == motor) {
    digitalWrite(FRONT_AIN_1, inPin1);
    digitalWrite(FRONT_AIN_2, inPin2);
    analogWrite(FRONT_PWM_A, speed);
  } else if (1 == motor) {
    digitalWrite(FRONT_BIN_1, inPin1);
    digitalWrite(FRONT_BIN_2, inPin2);
    analogWrite(FRONT_PWM_B, speed);
  }
}

// 後輪を動かす
void moveRear(int motor, int speed, int direction)
{  
  digitalWrite(REAR_STBY, HIGH);
  
  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;
  
  if (1 == direction) {
    inPin1 = HIGH;
    inPin2 = LOW;
  }
  
  if (0 == motor) {
    digitalWrite(REAR_AIN_1, inPin1);
    digitalWrite(REAR_AIN_2, inPin2);
    analogWrite(REAR_PWM_A, speed);
  } else if (1 == motor) {
    digitalWrite(REAR_BIN_1, inPin1);
    digitalWrite(REAR_BIN_2, inPin2);
    analogWrite(REAR_PWM_B, speed);
  }
}

// 前輪をストップさせる
void stopFront()
{
//enable standby  
  digitalWrite(FRONT_STBY, LOW); 
}

// 後輪をストップさせる
void stopRear()
{
  digitalWrite(REAR_STBY, LOW);
}

// 障害物を回避する
void avoid()
{
  Serial.println("avoid");
  bAvoided = true;
}

// Arduinoへコマンドを送信し、叫び声を再生する
void playShoutSound()
{
  Serial.println("shout");
}

// Arduinoへコマンドを送信し、歓喜の歌を再生する

// KONASHIの4ピンを監視する
void changeStateByKonashi()
{
  currentState = HIGH == digitalRead(KONASHI_PIN_5) ? STATE_DISABLE_CONSUME : STATE_ENABLE_CONSUME;
  
  if (HIGH == currentState) {
    Serial.println("consumable");
  } else {
    Serial.println("not consumable");
    
    bAvoided = false;
  }
}

// 測距センサーと曲げセンサーの値を監視する
void observeInputs()
{
//  Serial.println("check analog values");
  
  konashiPin4Value = digitalRead(KONASHI_PIN_4);
  konashiPin5Value = digitalRead(KONASHI_PIN_5);
  irSensorValue = analogRead(IR_SENSOR_PIN);  
  flexSensorValue = analogRead(FLEX_SENSOR_PIN);
//  Serial.print("IR: ");
//  Serial.print(irSensorValue);
//  Serial.print(", FLEX: ");
//  Serial.print(flexSensorValue);
//  Serial.print(", STATE: ");
//  Serial.println(currentState);
  
  if (LOW == konashiPin4PrevValue && HIGH == konashiPin4Value) {
    currentState = STATE_ENABLE_CONSUME;
  }
  
  if (LOW == konashiPin5PrevValue && HIGH == konashiPin5Value) {
    currentState = STATE_DISABLE_CONSUME;
  }
  
  if (konashiPin4Value == konashiPin5Value) {
    currentState = STATE_NEUTRAL;
  }
  
  if (prevState != currentState) {
    Serial1.print((char)START_BYTE);
    
    if (STATE_ENABLE_CONSUME == currentState) {
      Serial1.print((char)COMMAND_ENABLE_CONSUME);
    } else if (STATE_DISABLE_CONSUME == currentState) {
      Serial1.print((char)COMMAND_DISABLE_CONSUME);
    } else {
      // i.e. currentState = STATE_NEUTRAL
      Serial1.print((char)COMMAND_NEUTRAL);
    }
    
    Serial1.print((char)END_BYTE);
    
    prevState = currentState;
  }
  
  konashiPin4PrevValue = konashiPin4Value;
  konashiPin5PrevValue = konashiPin5Value;
  
  if (STATE_DISABLE_CONSUME == currentState && IR_SENSOR_THRESHOLD <= irSensorValue) {
    enableAvoidance = true;
  } else {
    enableAvoidance = false;
  }
  
  if (STATE_DISABLE_CONSUME == currentState && FLEX_SENSOR_THRESHOLD <= flexSensorValue) {
    playShoutSound();
  }
}
