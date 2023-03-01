//***********************************************
//            AUTOMATIC CHESSBOARD
// -- ESTAMOS TRABANJANDO EN ESTE @MM
//***********************************************
//******************************  INCLUDING FILES
#include "global.h"
#include "Micro_Max.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // In Skolodi Case LCD is on 0x27 and not in 0x20
//****************************************  SETUP
void setup() {
  Serial.begin(9600);
  //  ELECTROMAGNET
  pinMode (MAGNET, OUTPUT);
  //digitalWrite(MAGNET, HIGH); // prueba
  //delay (10000);
  //digitalWrite(MAGNET, LOW); // prueba
  //  MOTOR
  pinMode (MOTOR_WHITE_STEP, OUTPUT);
  pinMode (MOTOR_WHITE_DIR, OUTPUT);
  pinMode (MOTOR_BLACK_STEP, OUTPUT);
  pinMode (MOTOR_BLACK_DIR, OUTPUT);
  //  MULTIPLEXER
  for (byte i = 0; i < 4; i++) {
    pinMode (MUX_ADDR [i], OUTPUT);
    digitalWrite(MUX_ADDR [i], LOW);
    pinMode (MUX_SELECT [i], OUTPUT);
    digitalWrite(MUX_SELECT [i], HIGH);
  }
  pinMode (MUX_OUTPUT, INPUT_PULLUP);
  //  SET THE REED SWITCHES STATUS
  for (byte i = 2; i < 6; i++) {
    for (byte j = 0; j < 8; j++) {
      reed_sensor_status[i][j] = 1;
      reed_sensor_status_memory[i][j] = 1;
    }
  }
  //  MICROMAX CHESS APP 
  lastH[0] = 0;
  //  LCD
  lcd.init();
  //  COUNTDOWN
  timer = millis();
  //  ARCADE BUTTON - LIMIT SWITCH
  pinMode (BUTTON_WHITE_SWITCH_MOTOR_WHITE, INPUT_PULLUP);
  pinMode (BUTTON_BLACK_SWITCH_MOTOR_BLACK, INPUT_PULLUP);
  lcd_display();
}
//*****************************************  LOOP
void loop() {
  switch (sequence) {
    case start:
        Serial.println("loop->sequence->start");
        lcd_display();
        if (button(WHITE) == true) {  // HvsH Mode
          game_mode = HvsH;
          sequence = player_white;
        }else if (button(BLACK) == true) {  // HvsC Mode
          game_mode = HvsC;
          sequence = calibration;
        }
    break;
    case calibration:
        Serial.println("loop->sequence->calibration");
        lcd_display();
        calibrate();
        sequence = player_white;
    break;
    case player_white:
      Serial.println("loop->sequence->player_white");
        if (millis() - timer > 995) {  // Display the white player clock
          countdown();
          lcd_display();
        }
        detect_human_movement();
        if (button(WHITE) == true) {  // White player end turn
          new_turn_countdown = true;
          player_displacement();
          if (game_mode == HvsH) {
            AI_HvsH();  // Chekc is movement is valid
            if (no_valid_move == false) sequence = player_black;
            else lcd_display();
          } else if (game_mode == HvsC) {
            AI_HvsC();
            sequence = player_black;
          }
        } // bracket creado -- este bracket estaba al final de la fn loop
    break;
    case player_black:
      //  GAME MODE HVSH
      Serial.println("loop->sequence->player_black");
      if (game_mode == HvsH) {  // Display the black player clock
        if (millis() - timer > 995) {
          countdown();
          lcd_display();
        }
        detect_human_movement();
        if (button(BLACK) == true) {  // Black human player end turn
          new_turn_countdown = true;
          player_displacement();
          AI_HvsH();  // Chekc is movement is valid
          if (no_valid_move == false) sequence = player_white;
          else lcd_display();
        }
      }
      //  GAME MODE HVSC
      else if (game_mode == HvsC) {
        black_player_movement();  //  MOVE THE BLACK CHESS PIECE
        sequence = player_white;
      }
    break;
  }
//} bracket eliminado @MM
}
//***************************************  SWITCH
  boolean button(byte type) {
    Serial.println("fn->button");
    if (type == WHITE && digitalRead(BUTTON_WHITE_SWITCH_MOTOR_WHITE) != HIGH) {
      delay(250);
      return true;
    }
    if (type == BLACK && digitalRead(BUTTON_BLACK_SWITCH_MOTOR_BLACK) != HIGH) {
      delay(250);
      return true;
    }
    return false;
  }
//************************************  CALIBRATE
  void calibrate() {
    Serial.println("fn->calibrate");
  //  SLOW DISPLACEMENTS UP TO TOUCH THE LIMIT SWITCHES
    while (digitalRead(BUTTON_WHITE_SWITCH_MOTOR_WHITE) == HIGH) motor(B_T, SPEED_SLOW, calibrate_speed);
    while (digitalRead(BUTTON_BLACK_SWITCH_MOTOR_BLACK) == HIGH) motor(L_R, SPEED_SLOW, calibrate_speed);
    delay(500);
  //  RAPID DISPLACEMENTS UP TO THE BLACK START POSITION (E7)
    motor(R_L, SPEED_FAST, TROLLEY_START_POSITION_X);
    motor(T_B, SPEED_FAST, TROLLEY_START_POSITION_Y);
    delay(500);
  }
//****************************************  MOTOR
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
// *******************************  ELECTROMAGNET
  void electromagnet(boolean state) {
    Serial.println("fn->electromagnet");
    if (state == true)  {
      digitalWrite(MAGNET, HIGH);
      delay(600);
    } else  {
      delay(600);
      digitalWrite(MAGNET, LOW);
    }
  }
// ***********************************  COUNTDONW
  void countdown() {
    Serial.println("fn->countdown");
    //  SET THE TIME OF THE CURRENT PLAYER
      if (new_turn_countdown == true ) {
        new_turn_countdown = false;
        if (sequence == player_white) {
          second = second_white;
          minute = minute_white;
        }else if (sequence == player_black) {
          second = second_black;
          minute = minute_black;
        }
      }
    //  COUNTDOWN
      timer = millis();
      second = second - 1;
      if (second < 1) {
        second = 60;
        minute = minute - 1;
      }
    //  RECORD THE WHITE PLAYER TIME
      if (sequence == player_white) {
        second_white = second;
        minute_white = minute;
      }
    //  RECORD THE BLACK PLAYER TIME
      else if (sequence == player_black) {
        second_black = second;
        minute_black = minute;
      }
  }
// ***********************  BLACK PLAYER MOVEMENT
  void black_player_movement() {
    Serial.println("fn->black_player_movement");
    //  CONVERT THE AI CHARACTERS IN VARIABLES
      int departure_coord_X = lastM[0] - 'a' + 1;
      int departure_coord_Y = lastM[1] - '0';
      int arrival_coord_X = lastM[2] - 'a' + 1;
      int arrival_coord_Y = lastM[3] - '0';
      byte displacement_X = 0;
      byte displacement_Y = 0;
    //  TROLLEY DISPLACEMENT TO THE STARTING POSITION
      int convert_table [] = {0, 7, 6, 5, 4, 3, 2, 1, 0};
      byte white_capturing = 1;
      if (reed_sensor_status_memory[convert_table[arrival_coord_Y]][arrival_coord_X - 1] == 0) white_capturing = 0;
      for (byte i = white_capturing; i < 2; i++) {
        if (i == 0) {
          displacement_X = abs(arrival_coord_X - trolley_coordinate_X);
          displacement_Y = abs(arrival_coord_Y - trolley_coordinate_Y);
        }
        else if (i == 1) {
          displacement_X = abs(departure_coord_X - trolley_coordinate_X);
          displacement_Y = abs(departure_coord_Y - trolley_coordinate_Y);
        }
        if (departure_coord_X > trolley_coordinate_X) motor(T_B, SPEED_FAST, displacement_X);
        else if (departure_coord_X < trolley_coordinate_X) motor(B_T, SPEED_FAST, displacement_X);
        if (departure_coord_Y > trolley_coordinate_Y) motor(L_R, SPEED_FAST, displacement_Y);
        else if (departure_coord_Y < trolley_coordinate_Y) motor(R_L, SPEED_FAST, displacement_Y);
        if (i == 0) {
          electromagnet(true);
          motor(R_L, SPEED_SLOW, 0.5);
          motor(B_T, SPEED_SLOW, arrival_coord_X - 0.5);
          electromagnet(false);
          motor(L_R, SPEED_FAST, 0.5);
          motor(T_B, SPEED_FAST, arrival_coord_X - 0.5);
          trolley_coordinate_X = arrival_coord_X;
          trolley_coordinate_Y = arrival_coord_Y;
        }
      }
      trolley_coordinate_X = arrival_coord_X;
      trolley_coordinate_Y = arrival_coord_Y;
    //  MOVE THE BLACK CHESS PIECE TO THE ARRIVAL POSITION
      displacement_X = abs(arrival_coord_X - departure_coord_X);
      displacement_Y = abs(arrival_coord_Y - departure_coord_Y);
      electromagnet(true);
    //  BISHOP DISPLACEMENT
      if (displacement_X == 1 && displacement_Y == 2 || displacement_X == 2 && displacement_Y == 1) {
        if (displacement_Y == 2) {
          if (departure_coord_X < arrival_coord_X) {
            motor(T_B, SPEED_SLOW, displacement_X * 0.5);
            if (departure_coord_Y < arrival_coord_Y) motor(L_R, SPEED_SLOW, displacement_Y);
            else motor(R_L, SPEED_SLOW, displacement_Y);
            motor(T_B, SPEED_SLOW, displacement_X * 0.5);
          }else if (departure_coord_X > arrival_coord_X) {
            motor(B_T, SPEED_SLOW, displacement_X * 0.5);
            if (departure_coord_Y < arrival_coord_Y) motor(L_R, SPEED_SLOW, displacement_Y);
            else motor(R_L, SPEED_SLOW, displacement_Y);
            motor(B_T, SPEED_SLOW, displacement_X * 0.5);
          }
        }else if (displacement_X == 2) {
          if (departure_coord_Y < arrival_coord_Y) {
            motor(L_R, SPEED_SLOW, displacement_Y * 0.5);
            if (departure_coord_X < arrival_coord_X) motor(T_B, SPEED_SLOW, displacement_X);
            else motor(B_T, SPEED_SLOW, displacement_X);
            motor(L_R, SPEED_SLOW, displacement_Y * 0.5);
          }else if (departure_coord_Y > arrival_coord_Y) {
            motor(R_L, SPEED_SLOW, displacement_Y * 0.5);
            if (departure_coord_X < arrival_coord_X) motor(T_B, SPEED_SLOW, displacement_X);
            else motor(B_T, SPEED_SLOW, displacement_X);
            motor(R_L, SPEED_SLOW, displacement_Y * 0.5);
          }
        }
      }
    //  DIAGONAL DISPLACEMENT
      else if (displacement_X == displacement_Y) {
        if (departure_coord_X > arrival_coord_X && departure_coord_Y > arrival_coord_Y) motor(RL_BT, SPEED_SLOW, displacement_X);
        else if (departure_coord_X > arrival_coord_X && departure_coord_Y < arrival_coord_Y) motor(LR_BT, SPEED_SLOW, displacement_X);
        else if (departure_coord_X < arrival_coord_X && departure_coord_Y > arrival_coord_Y) motor(RL_TB, SPEED_SLOW, displacement_X);
        else if (departure_coord_X < arrival_coord_X && departure_coord_Y < arrival_coord_Y) motor(LR_TB, SPEED_SLOW, displacement_X);
      }
    //  KINGSIDE/QUEENSIDE CASTLING
      else if (departure_coord_X == 5 && departure_coord_Y == 8 && arrival_coord_X == 7 && arrival_coord_Y == 8) {  //  KINGSIDE CASTLING
        motor(R_L, SPEED_SLOW, 0.5);
        motor(T_B, SPEED_SLOW, 2);
        electromagnet(false);
        motor(T_B, SPEED_FAST, 1);
        motor(L_R, SPEED_FAST, 0.5);
        electromagnet(true);
        motor(B_T, SPEED_SLOW, 2);
        electromagnet(false);
        motor(T_B, SPEED_FAST, 1);
        motor(R_L, SPEED_FAST, 0.5);
        electromagnet(true);
        motor(L_R, SPEED_SLOW, 0.5);
      }else if (departure_coord_X == 5 && departure_coord_Y == 8 && arrival_coord_X == 3 && arrival_coord_Y == 8) {  //  QUEENSIDE CASTLING
        motor(R_L, SPEED_SLOW, 0.5);
        motor(B_T, SPEED_SLOW, 2);
        electromagnet(false);
        motor(B_T, SPEED_FAST, 2);
        motor(L_R, SPEED_FAST, 0.5);
        electromagnet(true);
        motor(T_B, SPEED_SLOW, 3);
        electromagnet(false);
        motor(B_T, SPEED_FAST, 1);
        motor(R_L, SPEED_FAST, 0.5);
        electromagnet(true);
        motor(L_R, SPEED_SLOW, 0.5);
      }
    //  HORIZONTAL DISPLACEMENT
      else if (displacement_Y == 0) {
        if (departure_coord_X > arrival_coord_X) motor(B_T, SPEED_SLOW, displacement_X);
        else if (departure_coord_X < arrival_coord_X) motor(T_B, SPEED_SLOW, displacement_X);
      }
    //  VERTICAL DISPLACEMENT
      else if (displacement_X == 0) {
        if (departure_coord_Y > arrival_coord_Y) motor(R_L, SPEED_SLOW, displacement_Y);
        else if (departure_coord_Y < arrival_coord_Y) motor(L_R, SPEED_SLOW, displacement_Y);
      }
    electromagnet(false);
    //  UPADTE THE REED SENSORS STATES WITH THE BALCK MOVE
    reed_sensor_status_memory[convert_table[departure_coord_Y]][departure_coord_X - 1] = 1;
    reed_sensor_status_memory[convert_table[arrival_coord_Y]][arrival_coord_X - 1] = 0;
    reed_sensor_status[convert_table[departure_coord_Y]][departure_coord_X - 1] = 1;
    reed_sensor_status[convert_table[arrival_coord_Y]][arrival_coord_X - 1] = 0;
  }
//**********************************  LCD DISPLAY
  void lcd_display() {
    Serial.println("fn->lcd_display");
    lcd.backlight();
    if (no_valid_move == true) {
      lcd.setCursor(0, 0);
      lcd.print("  NO VALID MOVE  ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(2000);
      no_valid_move = false;
      return;
    }
    switch (sequence) {
      case start_up:
        lcd.setCursor(0, 0);
        lcd.print("   AUTOMATIC    ");
        lcd.setCursor(0, 1);
        lcd.print("   CHESSBOARD   ");
        sequence = start;
        delay(4000);
      case start:
        lcd.setCursor(0, 0);
        lcd.print(" PRESS W - HvsH ");
        lcd.setCursor(0, 1);
        lcd.print(" PRESS B - HvsC ");
        break;
      case calibration:
        lcd.setCursor(0, 0);
        lcd.print("  CALIBRATION   ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        break;
      case player_white:
        lcd.setCursor(0, 0);
        lcd.print("     WHITE      ");
        lcd.setCursor(0, 1);
        lcd.print("     " + String(minute) + " : " + String(second) + "     ");
        break;
      case player_black:
      lcd.setCursor(0, 0);
      lcd.print("     BLACK      ");
      lcd.setCursor(0, 1);
      lcd.print("     " + String(minute) + " : " + String(second) + "     ");
      break;
    }
  }
//************************  DETECT HUMAN MOVEMENT
  void detect_human_movement() {
    Serial.println("fn->detect_human_movement");
    //  RECORD THE REED SWITCHES STATUS
    byte column = 6;
    byte row = 0;
    for (byte i = 0; i < 4; i++) {
      digitalWrite(MUX_SELECT[i], LOW);
      for (byte j = 0; j < 16; j++) {
        for (byte k = 0; k < 4; k++) {
          digitalWrite(MUX_ADDR [k], MUX_CHANNEL [j][k]);
          delay(5);                       // SPK Added for Recomendation on Questions See shuttle123
        }
        reed_sensor_record[column][row] = digitalRead(MUX_OUTPUT);
        row++;
        if (j == 7) {
          column++;
          row = 0;
        }
      }
      for (byte l = 0; l < 4; l++) {
        digitalWrite(MUX_SELECT[l], HIGH);
      }
      if (i == 0) column = 4;
      if (i == 1) column = 2;
      if (i == 2) column = 0;
      row = 0;
    }
    for (byte i = 0; i < 8; i++) {
      for (byte j = 0; j < 8; j++) {
        reed_sensor_status_memory[7 - i][j] = reed_sensor_record[i][j];
      }
    }
    //  COMPARE THE OLD AND NEW STATUS OF THE REED SWITCHES
    for (byte i = 0; i < 8; i++) {
      for (byte j = 0; j < 8; j++) {
        if (reed_sensor_status[i][j] != reed_sensor_status_memory[i][j]) {
          if (reed_sensor_status_memory[i][j] == 1) {
            reed_colone[0] = i;
            reed_line[0] = j;
          }
          if (reed_sensor_status_memory[i][j] == 0) {
            reed_colone[1] = i;
            reed_line[1] = j;
          }
        }
      }
    }
    //  SET THE NEW STATUS OF THE REED SENSORS
    for (byte i = 0; i < 8; i++) {
      for (byte j = 0; j < 8; j++) {
        reed_sensor_status[i][j] = reed_sensor_status_memory[i][j];
      }
    }
  }
//**************************  PLAYER DISPLACEMENT
  void player_displacement() {
    Serial.println("fn->player_displacement");
    //  CONVERT THE REED SENSORS SWITCHES COORDINATES IN CHARACTERS
    char table1[] = {'8', '7', '6', '5', '4', '3', '2', '1'};
    char table2[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    mov[0] = table2[reed_line[0]];
    mov[1] = table1[reed_colone[0]];
    mov[2] = table2[reed_line[1]];
    mov[3] = table1[reed_colone[1]];
  }
