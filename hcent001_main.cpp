#include "timerISR.h"
#include "hcent001_helper.h"
#include "hcent001_periph.h"
#include "hcent001_SPIAVR.h"
#include "LCD.h"

//Debugging purposes
#include <SerialAtmega.h>
#include <Arduino.h>

#define NUM_TASKS 9 //Number of tasks being used

#define paddleWidth 10
#define paddleHeight 40
#define screenWidth 128
#define screenHeight 128
#define X_OFFSET 2
#define Y_OFFSET 1

//Global variables
int P1_position = 44;
int P2_position = 44;

unsigned int P1_movement = 0;
unsigned int P2_movement = 0;

// Ball position and velocity
int ball_x = screenWidth / 2;
int ball_y = screenHeight / 2;
int ball_dx = 1; // direction x: +1 = right, -1 = left
int ball_dy = 1; // direction y: +1 = down, -1 = up
const int BALL_RADIUS = 2;

unsigned int P1_score = 0;
unsigned int P2_score = 0;

bool begin_game = false;

bool goal_scored = false;

bool end_game = false;

bool kick_off = true;

volatile bool buzz_triggered = false;

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

//Define Periods for each task
const unsigned long JOYSTICK_P1_PERIOD = 10;
const unsigned long JOYSTICK_P2_PERIOD = 10;
const unsigned long LCD_PERIOD = 1;
const unsigned long P1_POSITION_UPDATE_PERIOD = 1;
const unsigned long P2_POSITION_UPDATE_PERIOD = 1;
const unsigned long BALL_PERIOD = 5;
const unsigned long BUZZER_PERIOD = 5;
const unsigned long START_BUTTON_PERIOD = 1;
const unsigned long RESULT_DISPLAY_PERIOD = 100;
const unsigned long GCD_PERIOD = 1;

task tasks[NUM_TASKS]; // declared task array with 5 tasks

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   
		if ( tasks[i].elapsedTime == tasks[i].period ) {           
			tasks[i].state = tasks[i].TickFct(tasks[i].state); 
			tasks[i].elapsedTime = 0;                          
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        
	}
}

enum Joystick_P1_States {Joystick_P1_SMStart, Joystick_P1_Read} Joystick_P1_State;
int Joystick_P1_Tick(int state){
  switch (Joystick_P1_State){
    //State transitions
    case Joystick_P1_SMStart:
      Joystick_P1_State = Joystick_P1_Read;
      break;
    case Joystick_P1_Read:
      Joystick_P1_State = Joystick_P1_Read;
      break; 
    default:break; 
  }
  switch (Joystick_P1_State){
    //State actions
    case Joystick_P1_Read:
      P1_movement = map_value_joystick(ADC_read(0));
      //Serial.print("P1 read: ");
      //Serial.print(ADC_read(0));
      //Serial.print("    ");
      //Serial.print("P1: ");
      //Serial.println(P1_movement);
    default:break;
  }
  return Joystick_P1_State;
}

enum Joystick_P2_States {Joystick_P2_SMStart, Joystick_P2_Read} Joystick_P2_State;
int Joystick_P2_Tick(int state){
  switch (Joystick_P2_State){
    //State transitions
    case Joystick_P2_SMStart:
      Joystick_P2_State = Joystick_P2_Read;
      break;
    case Joystick_P2_Read:
      Joystick_P2_State = Joystick_P2_Read;
      break; 
    default:break; 
  }
  switch (Joystick_P2_State){
    //State actions
    case Joystick_P2_Read:
      P2_movement = map_value_joystick(ADC_read(1));
      //Serial.print("P2 read: ");
      //Serial.println(ADC_read(1));
      //Serial.print("    ");
      //Serial.print("P2: ");
      //Serial.println(P2_movement);
      break;
    default:break;

  }
  return Joystick_P2_State;
}

enum LCD_States {LCD_SMStart, LCD_Reset, LCD_Init, LCD_Display} LCD_State;
int LCD_Tick(int state) {
  static int prev_P1_position = 255;
  static int prev_P2_position = 255;
  static int prev_ball_x = -1;
  static int prev_ball_y = -1;

  switch (LCD_State) {
    static bool first_time = true; // Static variable to track if it's the first time displaying
    static bool first_time_end_game = true; // Static variable to track if it's the first time displaying end game
    case LCD_SMStart:
      LCD_State = LCD_Reset;
      break;

    case LCD_Reset:
      SPI_INIT();
      Hardware_Reset();
      LCD_State = LCD_Init;
      break;

    case LCD_Init:
      ST7735_Init();
      fillScreen(0x0000); // Clear screen once
      LCD_State = LCD_Display;
      break;

    case LCD_Display:
      static unsigned int start_blink = 0;
      int radii[] = {14, 12, 10, 8, 6, 4, 2};
      int numSteps = sizeof(radii) / sizeof(radii[0]);

      if(!begin_game){
        start_blink++;
        drawStringScaledCentered("Pong Game", 64, 70, 0x07E0, 2); // White, scale 2
        if(start_blink <= 50){
          drawStringScaledCentered("Press to start", 64, 40, 0x0000, 1); // Yellow, scale 1
        }
        else if(start_blink > 50 && start_blink < 100){
          drawStringScaledCentered("Press to start", 64, 40, 0x07E0, 1); // White, scale 1
        }else if(start_blink >= 100){
          start_blink = 0; // Reset blink counter
        }
      }else if(end_game){
        if(first_time_end_game){
          fillScreen(0x0000); // Clear screen for end game display
          first_time_end_game = false; // Set to false after the first display
        }
        if(P1_score >= 5){
          drawStringScaledCentered("Player 1", 64, 64, 0x07E0, 2); // Green, scale 2
          drawStringScaledCentered("Wins!", 64, 30, 0x07E0, 2); // Green, scale 2
        }else if(P2_score >= 5){
          drawStringScaledCentered("Player 2", 64, 64, 0x07E0, 2); // Green, scale 2
          drawStringScaledCentered("Wins!", 64, 30, 0x07E0, 2); // Green, scale 2
        }
      }else{
        if(first_time){
          fillScreen(0x0000); // Clear screen once
          first_time = false; // Set to false after the first display
        }
        // Draw paddles
      if (prev_P1_position != P1_position) {
        // Determine direction of movement
        int dy_P1 = P1_position - prev_P1_position;

        if (dy_P1 > 0) {
          // Paddle moved down: erase top strip of old paddle
          drawRect(0, prev_P1_position, paddleWidth, dy_P1, 0x0000);
        } else if (dy_P1 < 0) {
          // Paddle moved up: erase bottom strip of old paddle
          drawRect(0, P1_position + paddleHeight, paddleWidth, -dy_P1, 0x0000);
        }
        // Draw new paddle
        drawRect(0, P1_position, paddleWidth, paddleHeight, 0xFFFF);
        prev_P1_position = P1_position;
      }
      else if(prev_P1_position == P1_position){
        // If paddle position hasn't changed, just redraw it to ensure visibility
        drawRect(0, P1_position, paddleWidth, paddleHeight, 0xFFFF);
      }

      if (prev_P2_position != P2_position) {
        // Determine direction of movement
        int dy_P2 = P2_position - prev_P2_position;

        if (dy_P2 > 0) {
          // Paddle moved down: erase top strip of old paddle
          drawRect(screenWidth - paddleWidth, prev_P2_position, paddleWidth, dy_P2, 0x0000);
        } else if (dy_P2 < 0) {
          // Paddle moved up: erase bottom strip of old paddle
          drawRect(screenWidth - paddleWidth, P2_position + paddleHeight, paddleWidth, -dy_P2, 0x0000);
        }
        // Draw new paddle
        drawRect(screenWidth - paddleWidth, P2_position, paddleWidth, paddleHeight, 0xFFFF);
        prev_P2_position = P2_position;
      }
      else if(prev_P2_position == P2_position){
        // If paddle position hasn't changed, just redraw it to ensure visibility
        drawRect(screenWidth - paddleWidth, P2_position, paddleWidth, paddleHeight, 0xFFFF);
      }

      // Draw ball
      if(kick_off){
          for (int i = 0; i < numSteps; ++i) {
            if (i > 0) {
                // Erase the entire previous ball (not just the outline)
                drawFilledCircle(64, 64, radii[i - 1], 0x0000);
            }
            // Draw the smaller ball
            drawFilledCircle(64, 64, radii[i], 0x07E0);

            _delay_ms(100);
        }
        //drawFilledCircle(64, 64, BALL_RADIUS, 0x0000); // Draw the initial ball
        _delay_ms(100);
        
        prev_ball_x = -1;
        prev_ball_y = -1;
        ball_x = 64;
        ball_y = 64;
        kick_off = false; // Reset kick_off after drawing the initial ball

      }else{
        if (prev_ball_x != -1 && prev_ball_y != -1) {
        drawFilledCircle(prev_ball_x, prev_ball_y, BALL_RADIUS, 0x0000); // erase old ball
        }
        drawFilledCircle(ball_x, ball_y, BALL_RADIUS, 0x07E0); // draw new ball
        prev_ball_x = ball_x;
        prev_ball_y = ball_y;
      }
      break;         
    }
    default:break;
  }
  return LCD_State;
}

enum P1_Position_Update_States {P1_Position_SMStart, P1_Position_Update} P1_Position_Update_State;
int P1_Position_Update_Tick(int state){
  switch(P1_Position_Update_State){
    case P1_Position_SMStart:
      P1_Position_Update_State = P1_Position_Update;
      break;
    case P1_Position_Update:
      P1_Position_Update_State = P1_Position_Update;
      break;
    default:break;
  }
  switch(P1_Position_Update_State){
    case P1_Position_Update:
      if (P1_movement == 1) {
          P1_position += 1; // Move paddle down
      } else if (P1_movement == 2) {
          P1_position -= 1; // Move paddle up
      }
      // Clamp the value between 0 and screenHeight - paddleHeight
      if (P1_position < 0) {
          P1_position = 0;
      } else if (P1_position > screenHeight - paddleHeight) {
          P1_position = screenHeight - paddleHeight;
      }
      break;
    default:break;
  }
return P1_Position_Update_State;
}

enum P2_Position_Update_States {P2_Position_SMStart, P2_Position_Update} P2_Position_Update_State;
int P2_Position_Update_Tick(int state){
  switch(P2_Position_Update_State){
    case P2_Position_SMStart:
      P2_Position_Update_State = P2_Position_Update;
      break;
    case P2_Position_Update:
      P2_Position_Update_State = P2_Position_Update;
      break;
    default:break;
  }
  switch(P2_Position_Update_State){
    case P2_Position_Update:
      if (P2_movement == 1) {
          P2_position += 1; // Move paddle down
      } else if (P2_movement == 2) {
          P2_position -= 1; // Move paddle up
      }
      // Clamp the value between 0 and screenHeight - paddleHeight
      if (P2_position < 0) {
          P2_position = 0;
      } else if (P2_position > screenHeight - paddleHeight) {
          P2_position = screenHeight - paddleHeight;
      }
      break;
    default:break;

  }
return P2_Position_Update_State;
}

enum Ball_States { Ball_SMStart, Ball_Wait_Screen, Ball_Kick_Off, Ball_Move } Ball_State;
int Ball_Tick(int state) {
    switch(state) {
        case Ball_SMStart:
            state = Ball_Wait_Screen;
            break;
        case Ball_Wait_Screen:
            // Wait for the screen to be initialized before moving the ball
            if (begin_game) {
                state = Ball_Move;
            }else {
                state = Ball_Wait_Screen;
            }
            break;
        case Ball_Kick_Off:
            if(kick_off) {
                P1_score = 0;
                P2_score = 0;
              state = Ball_Kick_Off;
            }
            else {
              state = Ball_Move;
            }
        case Ball_Move:
            bool bounced = false;
            // Handle goal scoring once per event
            if (!goal_scored) {
                if (ball_x >= screenWidth + X_OFFSET + BALL_RADIUS) {
                    P1_score++;
                    goal_scored = true;
                    ball_x = screenWidth / 2;
                    ball_y = screenHeight / 2;
                    ball_dx = -ball_dx;
                }
                else if (ball_x <= -BALL_RADIUS) {
                    P2_score++;
                    goal_scored = true;
                    ball_x = screenWidth / 2;
                    ball_y = screenHeight / 2;
                    ball_dx = -ball_dx;
                }
                if( P1_score >= 5 || P2_score >= 5) {
                    end_game = true; // End the game if a player reaches 5 points
                }
            } else {
                if (ball_x > 0 && ball_x < screenWidth) {
                    goal_scored = false;
                }
            }
            // Move ball
            ball_x += ball_dx;
            ball_y += ball_dy;
            // Bounce off top/bottom
            if (ball_y <= 0 || ball_y >= screenHeight - BALL_RADIUS) {
                ball_dy = -ball_dy;
                bounced = true; // Set bounced to true when bouncing off top/bottom 
            }
            // Paddle collisions
            if (ball_dx < 0) {
                if (ball_x <= X_OFFSET + paddleWidth &&
                    ball_y + BALL_RADIUS >= P1_position &&
                    ball_y <= P1_position + paddleHeight) {
                    ball_dx = -ball_dx;
                    bounced = true; 
                }
            } else {
                if (ball_x + BALL_RADIUS >= screenWidth - paddleWidth &&
                    ball_y + BALL_RADIUS >= P2_position &&
                    ball_y <= P2_position + paddleHeight) {
                    ball_dx = -ball_dx;
                    bounced = true; 
                }
            }
          buzz_triggered = bounced;
          break;
        default:break;
    }
    return state;  
}

enum Start_Button_States {Start_Button_SMStart, Start_Button_Not_Pressed, Start_Button_Pressed} Start_Button_State;
int Start_Button_Tick(int state) {
    static bool previous = false;  
    static bool actual = false; 

    switch(Start_Button_State) {
        case Start_Button_SMStart:
            actual = GetBit(PINC, 2);
            Start_Button_State = Start_Button_Not_Pressed;
            break;

        case Start_Button_Not_Pressed:
            if (previous == actual){
              previous = actual;
              actual = GetBit(PINC, 2);
              Start_Button_State = Start_Button_Not_Pressed;
            }else{
              previous = actual;
              actual = GetBit(PINC, 2);
              Start_Button_State = Start_Button_Pressed;
            }
            break;

        case Start_Button_Pressed:
            if(previous == actual){
              previous = actual;
              actual = GetBit(PINC, 2);
              Start_Button_State = Start_Button_Pressed;
            }else{
              previous = actual;
              actual = GetBit(PINC, 2);
              begin_game = true; // Start the game
              Start_Button_State = Start_Button_Not_Pressed;
            }
            break;
    }

    return Start_Button_State;
}

enum Result_Display_States {Result_Display_SMStart, Result_Display_Display, Result_Display_Winner} Result_Display_State;
int Result_Display_Tick(int state) {
  char bufferP1[6];
  char bufferP2[6];
  switch(Result_Display_State) {
    case Result_Display_SMStart:
      lcd_clear();
      Result_Display_State = Result_Display_Display;
      break;

    case Result_Display_Display:
        if(!begin_game){
          lcd_goto_xy(0, 0);
          lcd_write_str("by");
          lcd_goto_xy(1, 0);
          lcd_write_str("Hugo CentenoSanz");
        }else{
          lcd_clear();
          // Display scores
          lcd_goto_xy(0, 0);
          lcd_write_str("P1 SCORE:");
          lcd_goto_xy(0, 10);
          itoa(P1_score, bufferP1, 10);
          for (int i = 0; bufferP1[i] != '\0'; i++) {
              lcd_write_character(bufferP1[i]);
          }
          
          lcd_goto_xy(1, 0);
          lcd_write_str("P2 SCORE:");
          lcd_goto_xy(1, 10);
                itoa(P2_score, bufferP2, 10);
          for (int i = 0; bufferP2[i] != '\0'; i++) {
              lcd_write_character(bufferP2[i]);
          }        
        }
        if(!end_game){
          Result_Display_State = Result_Display_Display;
        }else{
          Result_Display_State = Result_Display_Winner;
        }
        break;

    case Result_Display_Winner:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str("Thanks 4 playing!");
    break;
    default:break;
  }
  return Result_Display_State;
}

enum Buzzer_States {Buzzer_SMStart, Buzzer_Off, Buzzer_On} Buzzer_State;
int Buzzer_Tick(int state) {
  switch(Buzzer_State){
    static unsigned long buzz_counter = 0;
    case Buzzer_SMStart:
      Buzzer_State = Buzzer_Off;
      break;
    case Buzzer_Off:
      //PORTD &= ~(1 << PORTD1);
      //Serial.println("NO Buzzing");
      //Serial.println(PORTB);
      if(!buzz_triggered){
        Buzzer_State = Buzzer_Off;
      }else{
        Buzzer_State = Buzzer_On;
      }
      break;
    case Buzzer_On:
      //PORTD |= (1 << PORTD1);
      buzz_counter++;
      //Serial.println("Buzzing");
      //Serial.println(PORTB);
      if(buzz_counter < 1000){
        Buzzer_State = Buzzer_On;
      }else{
        buzz_counter = 0;
        Buzzer_State = Buzzer_Off; 
      }
      break;
    default:break;
  }
  return Buzzer_State;
}
      
int main(void) {
    //Debugging purposes
    //Serial.begin(9600);

    DDRC = 0b000000; PORTC = 0b111111;
    DDRB = 0b111111; PORTB = 0b000000;
    DDRD = 0b11111111; PORTD = 0b00000000;

    ADC_init();   
    lcd_init();
    lcd_clear();

    //Intialize tasks
    unsigned char i = 0;

    tasks[i].period = JOYSTICK_P1_PERIOD;
    tasks[i].state = Joystick_P1_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Joystick_P1_Tick;

    i++;
    tasks[i].period = JOYSTICK_P2_PERIOD;
    tasks[i].state = Joystick_P2_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Joystick_P2_Tick;
    
    i++;
    tasks[i].period = P1_POSITION_UPDATE_PERIOD;
    tasks[i].state = P1_Position_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &P1_Position_Update_Tick;

    i++;
    tasks[i].period = P2_POSITION_UPDATE_PERIOD;
    tasks[i].state = P2_Position_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &P2_Position_Update_Tick;

    i++;
    tasks[i].period = LCD_PERIOD;
    tasks[i].state = LCD_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &LCD_Tick;

    i++;
    tasks[i].period = BALL_PERIOD;
    tasks[i].state = Ball_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Ball_Tick;

    i++;
    tasks[i].period = START_BUTTON_PERIOD;
    tasks[i].state = Start_Button_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Start_Button_Tick;

    i++;
    tasks[i].period = RESULT_DISPLAY_PERIOD;
    tasks[i].state = Result_Display_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Result_Display_Tick;

    i++;
    tasks[i].period = BUZZER_PERIOD;
    tasks[i].state = Buzzer_SMStart;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Buzzer_Tick;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}
    return 0;
}