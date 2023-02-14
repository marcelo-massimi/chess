#include <Arduino.h>

//  Motor
const byte MOTOR_WHITE_DIR (2);
const byte MOTOR_WHITE_STEP (3);
const byte MOTOR_BLACK_DIR (4);
const byte MOTOR_BLACK_STEP (5);
const byte SQUARE_SIZE = 195;
const int SPEED_SLOW (3000);
const int SPEED_FAST (1000);

void setup() {
  Serial.begin(9600);
  //  MOTOR
  pinMode (MOTOR_WHITE_STEP, OUTPUT);
  pinMode (MOTOR_WHITE_DIR, OUTPUT);
  pinMode (MOTOR_BLACK_STEP, OUTPUT);
  pinMode (MOTOR_BLACK_DIR, OUTPUT);
  Serial.println("Setup OK")
}

void loop() {
  // put your main code here, to run repeatedly:
}