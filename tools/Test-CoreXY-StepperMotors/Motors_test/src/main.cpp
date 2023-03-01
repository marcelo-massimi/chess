#include <Arduino.h>
void motor(byte direction, int speed, float distance) {
  Serial.println("fn->calibrate");
  float step_number = 0;
  //  CALCUL THE DISTANCE
    if (distance == calibrate_speed) step_number = 4;
    else if (direction == LR_BT || direction == RL_TB || direction == LR_TB || direction == RL_BT) step_number = distance * SQUARE_SIZE * 1.44; //  ADD AN EXTRA LENGTH FOR THE DIAGONAL
    else step_number = distance * SQUARE_SIZE;
  //  DIRECTION OF THE MOTOR ROTATION
    if (direction == R_L || direction == T_B || direction == RL_TB) digitalWrite(MOTOR_WHITE_DIR, HIGH);
    else digitalWrite(MOTOR_WHITE_DIR, LOW);
    if (direction == B_T || direction == R_L || direction == RL_BT) digitalWrite(MOTOR_BLACK_DIR, HIGH);
    else digitalWrite(MOTOR_BLACK_DIR, LOW);
  //  ACTIVE THE MOTORS
    for (int x = 0; x < step_number; x++) {
      if (direction == LR_TB || direction == RL_BT) digitalWrite(MOTOR_WHITE_STEP, LOW);
      else digitalWrite(MOTOR_WHITE_STEP, HIGH);
      if (direction == LR_BT || direction == RL_TB) digitalWrite(MOTOR_BLACK_STEP, LOW);
      else digitalWrite(MOTOR_BLACK_STEP, HIGH);
      delayMicroseconds(speed);
      digitalWrite(MOTOR_WHITE_STEP, LOW);
      digitalWrite(MOTOR_BLACK_STEP, LOW);
      delayMicroseconds(speed);
    }
}
void calibrate() {
  Serial.println("fn->calibrate");
  //  RAPID DISPLACEMENTS UP TO THE BLACK START POSITION (E7)
  motor(R_L, SPEED_FAST, TROLLEY_START_POSITION_X);
  motor(T_B, SPEED_FAST, TROLLEY_START_POSITION_Y);
  delay(500);
}
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
  Serial.println("Setup OK");
}

void loop() {
  // put your main code here, to run repeatedly:

}
