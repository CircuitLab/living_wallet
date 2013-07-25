#include <MsTimer2.h>

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
#define IR_LED_PIN 13
volatile int irValue = 0;

#define FLEX_SENSOR_PIN A5
volatile int flexSensorValue = 0;

#define STATE_PIN 21
volatile int state = LOW;

void setup()
{
  Serial.begin(9600);
  
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
  
  pinMode(STATE_PIN, OUTPUT);
  attachInterrupt(4, blink, CHANGE); // PE6
//  attachInterrupt(2, readIR, CHANGE); // PD2
  
  pinMode(IR_LED_PIN, OUTPUT);
  MsTimer2::set(200, readIR);
  MsTimer2::start();
}

void loop()
{  
  moveFront(1, 255, 1); //motor 1, full speed, left
  moveFront(2, 255, 1); //motor 2, full speed, left

  delay(200); //go for 1 second
  stopFront(); //stop
  delay(250); //hold for 250ms until moveFront again

  moveFront(1, 255, 0); //motor 1, half speed, right
  moveFront(2, 255, 0); //motor 2, half speed, right

  delay(200);
  stopFront();
  delay(250);
  
  moveRear(1, 255, 1);
  moveRear(2, 255, 1);
  
  delay(200);
  stopRear();
  delay(250);
  
  moveRear(1, 255, 0); //motor 1, half speed, right
  moveRear(2, 255, 0); //motor 2, half speed, right
  
  delay(200);
  stopRear();
  delay(250);
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

void blink()
{
  state = !state;
  digitalWrite(STATE_PIN, state);
}

void readIR()
{
  irValue = analogRead(IR_SENSOR_PIN);
  analogWrite(IR_LED_PIN, irValue / 4);
  
  flexSensorValue = analogRead(FLEX_SENSOR_PIN);
  
  float realVoltageIR = irValue / 1024.0 * 5.0;
  float realVoltageFlex = flexSensorValue / 1024.0 * 5.0;
  Serial.print("IR Sensor: ");
  Serial.print(realVoltageIR);
  Serial.print(", Flex Sensor: ");
  Serial.println(realVoltageFlex);
}
