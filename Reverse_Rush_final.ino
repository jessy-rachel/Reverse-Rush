/*
  REVERSE RUSH — UNO R3 + MAX7219 + Joystick + Buzzer
  Mapping: Left/Right = SAME, Up/Down = OPPOSITE
*/

#include <LedControl.h>

/*** CONFIG ***/
const uint8_t PIN_MX_DIN = 12, PIN_MX_CLK = 11, PIN_MX_CS  = 10;
const uint8_t PIN_JOY_X  = A0, PIN_JOY_Y  = A1, PIN_JOY_SW = 2;
const uint8_t PIN_BUZZER = 8;
const int MATRIX_ORIENTATION = 0;              // 0/90/180/270
const unsigned long ROUND_TIME_MS = 900;
const unsigned long HOLD_OK_MS    = 180;
const uint8_t LIVES_START = 3;
int DEADZONE = 140;

const bool TWO_DIGIT_MIRROR = true;   // flip only the compact two-digit score if needed

/*** END CONFIG ***/

LedControl mx(PIN_MX_DIN, PIN_MX_CLK, PIN_MX_CS, 1);

enum Dir { NONE=-1, UP=0, DOWN=1, LEFT=2, RIGHT=3 };

// ---------- 8x8 icons
const byte ARROW_UP[8]    ={B00011000,B00011000,B00111100,B00111100,B01111110,B00011000,B00011000,B00011000};
const byte ARROW_DOWN[8]  ={B00011000,B00011000,B00011000,B01111110,B00111100,B00111100,B00011000,B00011000};
const byte ARROW_LEFT[8]  ={B00001000,B00001100,B00001110,B01111111,B01111111,B00001110,B00001100,B00001000};
const byte ARROW_RIGHT[8] ={B00010000,B00110000,B01110000,B11111110,B11111110,B01110000,B00110000,B00010000};
const byte TICK[8]        ={B00000000,B00000001,B00000011,B00000110,B10001100,B11011000,B01110000,B00100000};
const byte CROSS[8]       ={B10000001,B01000010,B00100100,B00011000,B00011000,B00100100,B01000010,B10000001};

// ---------- big digits (for single-digit score)
const byte DIGIT_0[8]={B00111100,B01100110,B01101110,B01110110,B01100110,B01100110,B00111100,B00000000};
const byte DIGIT_1[8]={B00011000,B00111000,B00011000,B00011000,B00011000,B00011000,B00111100,B00000000};
const byte DIGIT_2[8]={B00111100,B01100110,B00000110,B00001100,B00110000,B01100000,B01111110,B00000000};
const byte DIGIT_3[8]={B00111100,B01100110,B00000110,B00011100,B00000110,B01100110,B00111100,B00000000};
const byte DIGIT_4[8]={B00001100,B00011100,B00101100,B01001100,B01111110,B00001100,B00001100,B00000000};
const byte DIGIT_5[8]={B01111110,B01100000,B01111100,B00000110,B00000110,B01100110,B00111100,B00000000};
const byte DIGIT_6[8]={B00111100,B01100110,B01100000,B01111100,B01100110,B01100110,B00111100,B00000000};
const byte DIGIT_7[8]={B01111110,B00000110,B00001100,B00011000,B00110000,B00110000,B00110000,B00000000};
const byte DIGIT_8[8]={B00111100,B01100110,B01100110,B00111100,B01100110,B01100110,B00111100,B00000000};
const byte DIGIT_9[8]={B00111100,B01100110,B01100110,B00111110,B00000110,B01100110,B00111100,B00000000};

// ---------- tiny 3×5 font for side-by-side two digits
const byte FONT3x5[10][5] = {
  {B111,B101,B101,B101,B111}, //0
  {B010,B110,B010,B010,B111}, //1
  {B111,B001,B111,B100,B111}, //2
  {B111,B001,B111,B001,B111}, //3
  {B101,B101,B111,B001,B001}, //4
  {B111,B100,B111,B001,B111}, //5
  {B111,B100,B111,B101,B111}, //6
  {B111,B001,B010,B100,B100}, //7
  {B111,B101,B111,B101,B111}, //8
  {B111,B101,B111,B001,B111}  //9
};

// ---------- game state (define ONCE)
uint8_t lives = LIVES_START;
uint16_t score = 0;
Dir currentArrow = NONE;
int cx = 512, cy = 512;

// ---------- helpers
void muteBuzzer(){ noTone(PIN_BUZZER); digitalWrite(PIN_BUZZER, LOW); }
void beepOK(){ tone(PIN_BUZZER,1200,120); }
void beepFail(){ tone(PIN_BUZZER,220,200); }
void beepStart(){ tone(PIN_BUZZER,880,100); }

void rotatedShow(const byte img[8], int deg){
  byte out[8]={0,0,0,0,0,0,0,0};
  if (deg==0){ for(int r=0;r<8;r++) mx.setRow(0,r,img[r]); return; }
  for(int r=0;r<8;r++){
    for(int c=0;c<8;c++){
      if ((img[r]>>c)&1){
        int rr,cc;
        if (deg==90){ rr=c; cc=7-r; }
        else if (deg==180){ rr=7-r; cc=7-c; }
        else { rr=7-c; cc=r; } //270
        out[rr] |= (1<<cc);
      }
    }
  }
  for(int r=0;r<8;r++) mx.setRow(0,r,out[r]);
}
void matrixShow(const byte img[8]){ rotatedShow(img, MATRIX_ORIENTATION); }
void matrixShowBuf(const byte buf[8]){ rotatedShow(buf, MATRIX_ORIENTATION); }
void matrixClear(){ for(int r=0;r<8;r++) mx.setRow(0,r,0); }

void showDigitBig(int n){
  switch(n){
    case 0: matrixShow(DIGIT_0); break; case 1: matrixShow(DIGIT_1); break;
    case 2: matrixShow(DIGIT_2); break; case 3: matrixShow(DIGIT_3); break;
    case 4: matrixShow(DIGIT_4); break; case 5: matrixShow(DIGIT_5); break;
    case 6: matrixShow(DIGIT_6); break; case 7: matrixShow(DIGIT_7); break;
    case 8: matrixShow(DIGIT_8); break; case 9: matrixShow(DIGIT_9); break;
    default: matrixClear(); break;
  }
}

// draw two 3×5 digits on one 8×8 frame (handles left↔right mirroring)
void showTwoDigits(int tens, int ones){
  byte img[8] = {0,0,0,0,0,0,0,0};
  const int top  = 1;   // rows 1..5
  const int colT = 0;   // tens at cols 0..2
  const int colO = 4;   // ones at cols 4..6

  for(int r=0; r<5; r++){
    byte tr  = FONT3x5[tens][r];   // bits 2..0
    byte orr = FONT3x5[ones][r];

    for(int c=0; c<3; c++){
      if (tr & (1 << (2 - c))) {
        int col = TWO_DIGIT_MIRROR ? (7 - (colT + c)) : (colT + c);
        img[top + r] |= (1 << col);
      }
      if (orr & (1 << (2 - c))) {
        int col = TWO_DIGIT_MIRROR ? (7 - (colO + c)) : (colO + c);
        img[top + r] |= (1 << col);
      }
    }
  }
  matrixShowBuf(img);  // still honors MATRIX_ORIENTATION for your module
}


Dir oppositeOf(Dir d){
  if(d==UP) return DOWN; if(d==DOWN) return UP; if(d==LEFT) return RIGHT; if(d==RIGHT) return LEFT; return NONE;
}

Dir readJoystickDir(){
  int x=analogRead(PIN_JOY_X), y=analogRead(PIN_JOY_Y);
  int dx=x-cx, dy=y-cy;
  if (abs(dx)<DEADZONE && abs(dy)<DEADZONE) return NONE;
  if (abs(dx)>abs(dy)) return (dx<0)?LEFT:RIGHT;
  return (dy<0)?UP:DOWN;  // most sticks: UP = smaller Y
}

void showArrow(Dir d){
  switch(d){
    case UP:    matrixShow(ARROW_UP); break;
    case DOWN:  matrixShow(ARROW_DOWN); break;
    case LEFT:  matrixShow(ARROW_LEFT); break;
    case RIGHT: matrixShow(ARROW_RIGHT); break;
    default:    matrixClear(); break;
  }
}

Dir randomArrow(){ return (Dir)random(0,4); }

bool waitForOpposite(Dir requiredOpp, unsigned long timeLimitMs, unsigned long holdMs){
  unsigned long t0=millis(), okStart=0;
  while (millis()-t0 < timeLimitMs){
    Dir d=readJoystickDir();
    if (d==requiredOpp){
      if (!okStart) okStart=millis();
      if (millis()-okStart >= holdMs) return true;
    } else if (d==NONE){
      // keep waiting
    } else {
      okStart=0;
    }
  }
  return false;
}

void flashImage(const byte img[8], uint8_t times, uint16_t onMs, uint16_t offMs){
  for(uint8_t i=0;i<times;i++){ matrixShow(img); delay(onMs); matrixClear(); delay(offMs); }
}

void showScore(uint16_t s){
  tone(PIN_BUZZER,1100,120); delay(150);
  tone(PIN_BUZZER,1400,120); delay(120);
  muteBuzzer();

  uint16_t capped = (s>99)?99:s;
  if (capped < 10){
    showDigitBig(capped); delay(2200);
  } else {
    int tens=capped/10, ones=capped%10;
    showTwoDigits(tens, ones); delay(2500);
  }
  matrixClear();
}

void gameOver(){
  for(int i=0;i<3;i++) flashImage(CROSS,1,220,120);
  muteBuzzer();
  showScore(score);
  lives = LIVES_START;
  score = 0;
}

void startRound(){
  currentArrow = randomArrow();
  showArrow(currentArrow);
  beepStart();
}

void setup(){
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_JOY_SW,INPUT_PULLUP);
  mx.shutdown(0,false); mx.setIntensity(0,4); mx.clearDisplay(0);
  randomSeed(analogRead(A5));
  muteBuzzer();

  long sx=0, sy=0; const int N=50;
  for(int i=0;i<N;i++){ sx+=analogRead(PIN_JOY_X); sy+=analogRead(PIN_JOY_Y); delay(10); }
  cx = sx/N; cy = sy/N;
  DEADZONE = max(120, min(200, DEADZONE));

  startRound();
}

void loop(){
  if (currentArrow != NONE){
    Dir need = oppositeOf(currentArrow);
    bool ok = waitForOpposite(need, ROUND_TIME_MS, HOLD_OK_MS);

    if (ok){ score++; beepOK(); flashImage(TICK,1,180,80); }
    else   { if(lives>0) lives--; beepFail(); flashImage(CROSS,1,200,100); }
    muteBuzzer();

    if (lives==0){ gameOver(); startRound(); return; }
    startRound(); return;
  }

  // fallback idle
  muteBuzzer();
  matrixShow(ARROW_UP);    delay(150);
  matrixShow(ARROW_RIGHT); delay(150);
  matrixShow(ARROW_DOWN);  delay(150);
  matrixShow(ARROW_LEFT);  delay(150);
}
