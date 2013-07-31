#include <MsTimer2.h>

#define STATE_NEUTRAL 0
#define STATE_DISABLE_CONSUME 1
#define STATE_ENABLE_CONSUME 2

// MsTimer2でD3とD11を使用

#define STBY_FRONT 16 // PB6 : standby
#define STBY_REAR  19  // PF5

// Front Motor A
#define PWM_FRONT_A 10 //Speed control 
#define AIN_FRONT_1 14 //Direction
#define AIN_FRONT_2 8 //Direction

// Front Motor B
#define PWM_FRONT_B 5 //Speed control
#define BIN_FRONT_1 15 //Direction
#define BIN_FRONT_2 12 //Direction

// Rear Motor A
#define PWM_REAR_A 6
#define AIN_REAR_1 2
#define AIN_REAR_2 4

// Rear Motor B
#define PWM_REAR_B 9
#define BIN_REAR_1 17
#define BIN_REAR_2 18

#define IR_SENSOR_PIN A4
//#define IR_LED_PIN 13
volatile int irValue = 0;

#define FLEX_SENSOR_PIN A5
volatile int flexSensorValue = 0;

#define STATE_PIN 21
volatile int state = LOW;

#define KONASHI_PIN_4 13
#define KONASHI_PIN_5 7
volatile int konashiPin4PrevValue = LOW;
volatile int konashiPin5PrevValue = LOW;
volatile int konashiPin4Value = LOW;
volatile int konashiPin5Value = LOW;

boolean isConsumable = false;
volatile int currentState;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  
  analogReference(INTERNAL);
  
  // setup front motors
  pinMode(STBY_FRONT, OUTPUT);

  pinMode(PWM_FRONT_A, OUTPUT);
  pinMode(AIN_FRONT_1, OUTPUT);
  pinMode(AIN_FRONT_2, OUTPUT);

  pinMode(PWM_FRONT_B, OUTPUT);
  pinMode(BIN_FRONT_1, OUTPUT);
  pinMode(BIN_FRONT_2, OUTPUT);
  
  // setup rear motors
  pinMode(STBY_REAR, OUTPUT);
  
  pinMode(PWM_REAR_A, OUTPUT);
  pinMode(AIN_REAR_1, OUTPUT);
  pinMode(AIN_REAR_2, OUTPUT);
  
  pinMode(PWM_REAR_B, OUTPUT);
  pinMode(BIN_REAR_1, OUTPUT);
  pinMode(BIN_REAR_2, OUTPUT);
  
  pinMode(KONASHI_PIN_4, INPUT);
  pinMode(KONASHI_PIN_5, INPUT);
    
  konashiPin4PrevValue = digitalRead(KONASHI_PIN_4);
  konashiPin5PrevValue = digitalRead(KONASHI_PIN_5);
  
  currentState = STATE_NEUTRAL;
  
  MsTimer2::set(200, observeInputs);
  MsTimer2::start();
}

void loop()
{
  if (STATE_ENABLE_CONSUME == currentState) {
    moveFront(1, 255, 1);
    moveFront(1, 255, 1);
    delay(300);
    stopFront();
    
    moveRear(1, 255, 1);
    moveRear(2, 255, 1);
    delay(300);
    stopRear();
  } else if (STATE_DISABLE_CONSUME == currentState) {
    
  }
  
//  moveFront(1, 255, 1); //motor 1, full speed, left
//  moveFront(2, 255, 1); //motor 2, full speed, left
//
//  delay(200); //go for 1 second
//  stopFront(); //stop
//  delay(250); //hold for 250ms until moveFront again
//
//  moveFront(1, 255, 0); //motor 1, half speed, right
//  moveFront(2, 255, 0); //motor 2, half speed, right
//
//  delay(200);
//  stopFront();
//  delay(250);
//  
//  moveRear(1, 255, 1);
//  moveRear(2, 255, 1);
//  
//  delay(200);
//  stopRear();
//  delay(250);
//  
//  moveRear(1, 255, 0); //motor 1, half speed, right
//  moveRear(2, 255, 0); //motor 2, half speed, right
//  
//  delay(200);
//  stopRear();
//  delay(250);
}


void moveFront(int motor, int speed, int direction)
{
//move specific motor at speed and direction
//motor: 0 for B 1 for A
//speed: 0 is off, and 255 is full speed
//direction: 0 clockwise, 1 counter-clockwise  
  digitalWrite(STBY_FRONT, HIGH); //disable standby
  

  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if(1 == direction) {
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if(1 == motor) {
    digitalWrite(AIN_FRONT_1, inPin1);
    digitalWrite(AIN_FRONT_2, inPin2);
    analogWrite(PWM_FRONT_A, speed);
  } else {
    digitalWrite(BIN_FRONT_1, inPin1);
    digitalWrite(BIN_FRONT_2, inPin2);
    analogWrite(PWM_FRONT_B, speed);
  }
}

void moveRear(int motor, int speed, int direction)
{  
  digitalWrite(STBY_REAR, HIGH);
  
  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;
  
  if (1 == direction) {
    inPin1 = HIGH;
    inPin2 = LOW;
  }
  
  if (1 == motor) {
    digitalWrite(AIN_REAR_1, inPin1);
    digitalWrite(AIN_REAR_2, inPin2);
    analogWrite(PWM_REAR_A, speed);
  } else {
    digitalWrite(BIN_REAR_1, inPin1);
    digitalWrite(BIN_REAR_2, inPin2);
    analogWrite(PWM_REAR_B, speed);
  }
}

void stopFront()
{
//enable standby  
  digitalWrite(STBY_FRONT, LOW); 
}

void stopRear()
{
  digitalWrite(STBY_REAR, LOW);
}

void observeInputs()
{
  irValue = analogRead(IR_SENSOR_PIN);  
  flexSensorValue = analogRead(FLEX_SENSOR_PIN);
  
  // konashiの4、5番ピンを監視する
  konashiPin4Value = digitalRead(KONASHI_PIN_4);
  konashiPin5Value = digitalRead(KONASHI_PIN_5);
  
  if (LOW == konashiPin4PrevValue && HIGH == konashiPin4Value) {
    Serial.println("konashi pin 4 is rising");
    Serial1.println("pin 4 rising");
    
    currentState = STATE_ENABLE_CONSUME;
  } else if (HIGH == konashiPin4PrevValue && LOW == konashiPin4Value) {
    Serial.println("konashi pin 4 is falling");
    Serial1.println("pin 4 falling");
  }
  
  if (LOW == konashiPin5PrevValue && HIGH == konashiPin5Value) {
    Serial.println("konashi pin 5 is rising");
    Serial1.println("pin 5 rising");
    
    currentState = STATE_DISABLE_CONSUME;
  } else if (HIGH == konashiPin5PrevValue && LOW == konashiPin5Value) {
    Serial.println("konashi pin 5 is falling");
    Serial1.println("pin 5 falling");
  }
  
  if (HIGH == konashiPin4Value && HIGH == konashiPin5Value) {
    currentState = STATE_NEUTRAL;
  }
  
  konashiPin4PrevValue = konashiPin4Value;
  konashiPin5PrevValue = konashiPin5Value;
  
  float realVoltageIR = irValue / 1024.0 * 5.0;
  float realVoltageFlex = flexSensorValue / 1024.0 * 5.0;
//  Serial.print("IR Sensor: ");
//  Serial.print(realVoltageIR);
//  Serial.print(", Flex Sensor: ");
//  Serial.print(realVoltageFlex);
//  Serial.print(", KONASHI pin 4: ");
//  Serial.print(konashiPin4Value);
//  Serial.print(", KONASHI pin 5: ");
//  Serial.println(konashiPin5Value);
}

void serialEvent()
{
  Serial.println("serial event");
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    Serial.print("serial input: ");
    Serial.println(inChar);
  }
}
