// Paint example specifically for the TFTLCD breakout board.
// If using the Arduino shield, use the tftpaint_shield.pde sketch instead!
// DOES NOT CURRENTLY WORK ON ARDUINO LEONARDO

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
//#include <TFT.h>            // Arduino TFT library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

//#define __TOUCHDEBUG__

#include "defines.h"

#define VERSION "V0.05"

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29

// For the Arduino Due, use digital pins 33 through 40
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 33
//   D1 connects to digital pin 34
//   D2 connects to digital pin 35
//   D3 connects to digital pin 36
//   D4 connects to digital pin 37
//   D5 connects to digital pin 38
//   D6 connects to digital pin 39
//   D7 connects to digital pin 40


// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

int oldcolor, currentcolor;

int giStepCnt = 0;
int giLoopCnt = 0;
int giMaxLoop = 10;
int giTargetPos = 0;
int giHubSteps = 0;

typedef enum State { NOTHING, HOMING, HOME, UP, DOWN, RUN, STOP } tStates;

tStates glState = NOTHING;

unsigned char g_ucMotorEnable = 0;

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
int PlaceButton(int iX, int iY, char *szText, int iColor)
{

  tft.setTextSize(2);

  tft.fillRect(iX, iY, BUTTONSIZE_X, BUTTONSIZE_Y, iColor);
  tft.drawRect(iX, iY, BUTTONSIZE_X, BUTTONSIZE_Y, WHITE);
  tft.setCursor(iX + 10 , iY + 15);
  tft.print(szText);

  return 0;
} /* PlaceButton */

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
void setup(void) {
  Serial.begin(9600);
  
  tft.reset();
  
  uint16_t identifier = tft.readID();

  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    return;
  }




  tft.begin(identifier);

  drawScreen();


  pinMode(13, OUTPUT);
  
  pinMode(STEPSTEPPIN, OUTPUT);
  pinMode(STEPDIRPIN, OUTPUT);
  
  // Set the spinning direction CW/CCW:
  digitalWrite(STEPDIRPIN, LOW);

  pinMode(MOTORENABLE, INPUT);
  pinMode(HOMESWITCH,  INPUT);

  Serial.println("Achsensteuerung");
  Serial.println("Jochen Kunz - JKu");
  Serial.println("Micro-Epsilon Messtechnik");
  Serial.print("Version ");
  Serial.println(VERSION);
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  Serial.println("STEPDIRPIN  10");
  Serial.println("STEPSTEPPIN 11");
  Serial.println("MOTORENABLE 12");
  Serial.println("HOMESWITCH  13");
  Serial.println("started ...");

} /* setup */


/**********************************************************************/
/*                                                                    */
/**********************************************************************/
int drawScreen(void)
{

  int x = FIRSTBUTTON_X;
  int y = FIRSTBUTTON_Y;


  char szText[10];

  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(BLACK); 

  strcpy(szText, "Home");
  PlaceButton(x,y,szText,CYAN);

  strcpy(szText, "Fast");
  PlaceButton(x+BUTTONDIST_X,y,szText,WHITE);

  strcpy(szText, "Up");
  PlaceButton(x,y+BUTTONDIST_Y,szText,BLUE);

  strcpy(szText, "Down");
  PlaceButton(x+BUTTONDIST_X,y+BUTTONDIST_Y,szText,BLUE);

  strcpy(szText, "Start");
  PlaceButton(x,y+(BUTTONDIST_Y*2),szText,GREEN);

  strcpy(szText, "Stop");
  PlaceButton(x+BUTTONDIST_X,y+(50*2),szText,RED);

  strcpy(szText, "Save");
  PlaceButton(x,y+(BUTTONDIST_Y*3),szText,CYAN);
  strcpy(szText, "Leer2");
  PlaceButton(x+BUTTONDIST_X,y+(BUTTONDIST_Y*3),szText,WHITE);
 
  tft.setTextSize(2);
  tft.setCursor(100, 220);
  tft.print("Pos:");

  tft.setTextSize(2);
  tft.setCursor(200, 220);
  tft.print("Cnt:");

  updateSteps();
  updateStatus(NULL);

} /* drawScreen () */
 

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
int updateSteps(void)
{
  tft.setTextSize(2);
  tft.setTextColor(RED,BLACK);
  tft.setCursor(150, 220);

  if (giStepCnt < 1000) {
    tft.fillRect(150,215, 45, 20, BLACK);
  }

  tft.print(giStepCnt);
} /* updateSteps */

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
int updateLoopCnt(void)
{
  tft.setTextSize(2);

  if (giLoopCnt == 0) {
    tft.fillRect(250,215, 60, 20, BLACK);
  }

  tft.setTextColor(GREEN,BLACK);
  tft.setCursor(245, 220);

  tft.print(giLoopCnt);
} /* updateLoopCnt */

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
int updateStatus(char *szText)
{
  tft.setTextSize(2);
  tft.setTextColor(WHITE,BLACK);
  tft.setCursor(20, 220);
  //tft.fillRect(20,215, 100, 21, BLACK);

  if (szText) { 
    tft.print(szText);
  }
} /* updateStatus */



long myTimer = 0;
long myTimeout = 0;

unsigned char g_ucDir = HIGH;
unsigned char g_ucMotorRun = 0;


/**********************************************************************/
/*                                                                    */
/**********************************************************************/
void loop()
{
  ///digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  char szStatus[10];
  ///digitalWrite(13, LOW);
  
  g_ucMotorEnable = digitalRead(MOTORENABLE);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

/*
  if (digitalRead(HOMESWITCH) == 1) {
    Serial.println("Home 1");
  }
  if (digitalRead(MOTORENABLE) == 1) {
    Serial.println("MOTORENABLE 1");
  }
*/
  
  if (g_ucMotorEnable && g_ucMotorRun) {

    if (millis() > myTimeout + myTimer ) {
      myTimer = millis();

      digitalWrite(STEPSTEPPIN, HIGH);
      delayMicroseconds(PULSEDELAY);
      digitalWrite(STEPSTEPPIN, LOW);
      delayMicroseconds(PULSEDELAY);


      if (g_ucDir == LOW) {
        giStepCnt++;
      } else {
        giStepCnt--;
      }

/*
      if ((glState == DOWN) || (glState == UP)) {
        updateSteps();
      }
*/

      if ((glState != HOMING) && (giStepCnt <= 0)) {
        g_ucMotorRun = 0;
        updateSteps();
      }
      digitalWrite(STEPDIRPIN, g_ucDir);

      //updateSteps();
    }

    if (glState == HOMING) {
      g_ucDir = HIGH;
      
      if (digitalRead(HOMESWITCH) == 0) {
        Serial.println("Home");
        g_ucMotorRun = 0; 
        g_ucDir = LOW;
        giStepCnt = 0;
        glState = HOME;
        updateSteps();

        strcpy(szStatus,"Homed ");
        updateStatus(szStatus);
      }
    } else if (glState == RUN) {
      if (giStepCnt <= (giTargetPos - giHubSteps)) {
        //g_ucMotorRun = 0; 
        //glState = STOP;
        g_ucDir = LOW;
        updateSteps();
        if (giLoopCnt >= giMaxLoop) {
          glState = STOP;
          g_ucMotorRun = 0;
          updateStatus(szStatus);
        }
      } else if (giStepCnt >= giTargetPos) {
        g_ucDir = HIGH;
        updateSteps();
        giLoopCnt++;
        updateLoopCnt();
      }
    }
  }

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int iX = 0;
    int iY = 0;
   
    // scale from 0->1023 to tft.width

#ifdef __TOUCHDEBUG__ 
    Serial.print(p.x);
    Serial.print("/");
    Serial.print(p.y);
    Serial.print(" - ");
    Serial.print(iY);
    Serial.print("/");
    Serial.println(iX);
#endif

    iX = map(p.y, TS_MAXY, TS_MINY, 0, tft.width());
    iY = map(p.x, TS_MINY, TS_MAXY, tft.height(), 0);



#ifdef __TOUCHDEBUG__ 
    Serial.print("mapped: ");
    Serial.print(iY);
    Serial.print("/");
    Serial.println(iX);
#endif

    if ((iX > FIRSTBUTTON_X) && (iX < (FIRSTBUTTON_X + BUTTONSIZE_X)) &&
        (iY > FIRSTBUTTON_Y) && (iY < (FIRSTBUTTON_Y + BUTTONSIZE_Y)) ) {
      strcpy(szStatus,"Home  ");
      glState = HOMING;
      updateStatus(szStatus);
      g_ucMotorRun = 1;
    } else if ((iX > (FIRSTBUTTON_X + BUTTONDIST_X) ) &&
               (iX < (FIRSTBUTTON_X + BUTTONSIZE_X + BUTTONDIST_X)) &&
               (iY > FIRSTBUTTON_Y) &&
               (iY < (FIRSTBUTTON_Y + BUTTONSIZE_Y)) ) {
      strcpy(szStatus,"Fast");
      if (myTimeout != 0) {
         myTimeout = 0;
      } else {
         myTimeout = MYTIMEOUT;
      }
      Serial.print("myTimeout: ");
      Serial.println(myTimeout);
    } else if ((iX > FIRSTBUTTON_X) && (iX < (FIRSTBUTTON_X + BUTTONSIZE_X)) &&
               (iY > (FIRSTBUTTON_Y + BUTTONDIST_Y) ) &&
               (iY < (FIRSTBUTTON_Y + BUTTONSIZE_Y + BUTTONDIST_Y)) ) {
      if (glState != HOME) {
        strcpy(szStatus,"Up      ");
        if (glState != UP) {
          updateStatus(szStatus);
        }
        glState = UP;
        g_ucDir = HIGH;
        g_ucMotorRun = 1;
      }
    } else if ((iX > (FIRSTBUTTON_X + BUTTONDIST_X) ) &&
               (iX < (FIRSTBUTTON_X + BUTTONSIZE_X + BUTTONDIST_X)) &&
               (iY > (FIRSTBUTTON_Y + BUTTONDIST_Y) ) &&
               (iY < (FIRSTBUTTON_Y + BUTTONSIZE_Y + BUTTONDIST_Y)) ) {
      strcpy(szStatus,"Down  ");
      if (glState != DOWN) {
        updateStatus(szStatus);
      }
      g_ucMotorRun = 1;
      g_ucDir = LOW;
      glState = DOWN;
    } else if ((iX > FIRSTBUTTON_X) && (iX < (FIRSTBUTTON_X + BUTTONSIZE_X)) &&
               (iY > (FIRSTBUTTON_Y + (BUTTONDIST_Y*2)) ) &&
               (iY < (FIRSTBUTTON_Y + (BUTTONSIZE_Y*2) + BUTTONDIST_Y)) ) {
      // "Start"
      Serial.print("giStepCnt ");
      Serial.println(giStepCnt);
      Serial.print("giHubSteps ");
      Serial.println(giHubSteps);


      if (giStepCnt < HUBSTEPS) {
        giHubSteps = giStepCnt - 10;
      } else {
        giHubSteps = HUBSTEPS;
      }

      if (glState != RUN) {
        strcpy(szStatus,"Start ");
        updateStatus(szStatus);
        g_ucDir = HIGH;
        glState = RUN;
        g_ucMotorRun = 1;
      }
    } else if ((iX > (FIRSTBUTTON_X + BUTTONDIST_X) ) &&
               (iX < (FIRSTBUTTON_X + BUTTONSIZE_X + BUTTONDIST_X)) &&
               (iY > (FIRSTBUTTON_Y + (BUTTONDIST_Y*2)) ) &&
               (iY < (FIRSTBUTTON_Y + (BUTTONSIZE_Y*2) + BUTTONDIST_Y)) ) {
      strcpy(szStatus,"Stop  ");
      glState = STOP;
      g_ucMotorRun = 0;
      updateStatus(szStatus);
    } else if ((iX > FIRSTBUTTON_X) && (iX < (FIRSTBUTTON_X + BUTTONSIZE_X)) &&
               (iY > (FIRSTBUTTON_Y + (BUTTONDIST_Y*3)) ) &&
               (iY < (FIRSTBUTTON_Y + (BUTTONSIZE_Y*3) + BUTTONDIST_Y)) ) {
      strcpy(szStatus,"Save  ");
      giTargetPos = giStepCnt;
      giLoopCnt = 0;
      glState = STOP;
      g_ucMotorRun = 0;
      Serial.print("Position Saved ");
      Serial.println(giStepCnt);
      updateSteps();
      updateLoopCnt();
      updateStatus(szStatus);
    } else if ((iX > (FIRSTBUTTON_X + BUTTONDIST_X) ) &&
               (iX < (FIRSTBUTTON_X + BUTTONSIZE_X + BUTTONDIST_X)) &&
               (iY > (FIRSTBUTTON_Y + (BUTTONDIST_Y*3)) ) &&
               (iY < (FIRSTBUTTON_Y + (BUTTONSIZE_Y*3) + BUTTONDIST_Y)) ) {
      strcpy(szStatus,"Leer2");
    } else {
      strcpy(szStatus,"");
    }


/*
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(140, 220);
    tft.fillRect(140,215, 100, 20, BLACK);
    tft.print(iX);
    tft.print("/");
    tft.print(iY);
*/
    // touch gedrueckt 
  } else {
    if ((glState == UP) || (glState == DOWN)) {
      //updateSteps();
      g_ucMotorRun = 0;
      glState = STOP;
    }
  }

} /* loop() */
