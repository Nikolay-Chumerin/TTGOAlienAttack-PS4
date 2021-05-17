#define DONT_SHOW_INTRO_ANIMATION
#define USE_PS4_CONTROLLER

#include <SPI.h>
#include <TFT_eSPI.h>
#include "back.h"
#include "enemy.h"
#include "gameover.h"
#include "player.h"

#ifdef USE_PS4_CONTROLLER
#include "PS4Controller.h"
constexpr int8_t kPs4StickThreshold = 30;
#endif  // USE_PS4_CONTROLLER

#ifdef SHOW_INTRO_ANIMATION
#include "ani.h"
#endif  // SHOW_INTRO_ANIMATION

TFT_eSPI tft = TFT_eSPI();

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

#define TFT_GREY 0x5AEB

constexpr int kScreenWidth = 135;
constexpr int kScreenHeight = 240;

// int player
float pX = random(30, 100);
float pY = 200;
int pW = 25;
int pH = 31;
float pXs = 0.2;
float pYs = 0.2;
int pos[4] = {2, 35, 68, 101};

// bulet
float bx[10];
float by[10];
int bw = 2;       // player bullet size
float bs = 0.35;  // player bulett speed

// enemies

float ex[8];
float ey[8];
int ef[8];
int ew = 32;
int eh = 32;
float esy = 0.11;
float esx = 0;
int eFrame[8];
int el[8];

float pres1 = 0;  // debounce left button
float pres2 = 0;  // debounce right button
int counter = 0;
int fire = 0;
int score = 0;
int timeAlive = 0;
int currentTime = 0;
int newLevelTime = 15;

int count = 0;
int startCounter = 0;
int endCounter = 0;
int aniFrame = 0;
int phase = 0;  // phase 0=start screen 1=play 2=gameover
//-----------------------------------------------------------------------------
#ifdef USE_PS4_CONTROLLER
void init_ps4() { PS4.begin("03:03:03:03:03:03"); }
#endif  // USE_PS4_CONTROLLER
//-----------------------------------------------------------------------------
void setup() {
#ifdef USE_PS4_CONTROLLER
  init_ps4();
#endif  // USE_PS4_CONTROLLER
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 100);
  pinMode(0, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);

  for (int i = 0; i < 8; i++) {
    ex[i] = pos[random(0, 4)];
    ey[i] = -50;
    eFrame[i] = random(0, 4);
    el[i] = 3;
  }

  for (int i = 0; i < 10; ++i) {
    by[i] = kScreenHeight + 10;
  }
}  // setup()
//-----------------------------------------------------------------------------
bool left_button_pressed() {
  return (digitalRead(0) == 0)
#ifdef USE_PS4_CONTROLLER
         || (PS4.isConnected() &&
             (PS4.Left() || (PS4.LStickX() < -kPs4StickThreshold)))
#endif  // USE_PS4_CONTROLLER
      ;
}  // left_button_pressed()
//-----------------------------------------------------------------------------
bool right_button_pressed() {
  return (digitalRead(35) == 0)
#ifdef USE_PS4_CONTROLLER
         || (PS4.isConnected() &&
             (PS4.Right() || (PS4.LStickX() > kPs4StickThreshold)))
#endif  // USE_PS4_CONTROLLER
      ;
}  // right_button_pressed()
//-----------------------------------------------------------------------------
bool only_left_button_pressed() {
  return left_button_pressed() && !right_button_pressed();
}  // only_left_button_pressed()
//-----------------------------------------------------------------------------
bool only_right_button_pressed() {
  return !left_button_pressed() && right_button_pressed();
}  // only_right_button_pressed()
//-----------------------------------------------------------------------------
bool fire_button_pressed() {
  return (left_button_pressed() && right_button_pressed())
#ifdef USE_PS4_CONTROLLER
         || (PS4.isConnected() && PS4.Cross())
#endif  // USE_PS4_CONTROLLER
      ;
}  // fire_button_pressed()
//-----------------------------------------------------------------------------
void show_intro() {
  delay(500);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.pushImage(0, 0, kScreenWidth, kScreenHeight, back);
  while (!fire_button_pressed()) {
#ifdef SHOW_INTRO_ANIMATION
    tft.pushImage(34, 0, kStartAnimationWidth, kStartAnimationHeight,
                  ani[aniFrame]);
    aniFrame = (aniFrame + 1) % kStartAnimationFramesNum;
#endif  // SHOW_INTRO_ANIMATION
    delay(40);
  }
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, 0x7820);
  tft.fillRect(0, 0, kScreenWidth, 17, 0x7820);
  tft.drawString("SCORE:", 2, 1, 2);
  tft.drawString("TIME:", 70, 1, 2);
  tft.drawLine(0, 17, kScreenWidth, 17, 0x6577);
  tft.drawLine(0, 18, 0, kScreenHeight, 0x6577);
  tft.drawLine(kScreenWidth - 1, 18, kScreenWidth - 1, kScreenHeight, 0x6577);

  phase = 1;
}  // show_intro()
//-----------------------------------------------------------------------------
void draw() {
  tft.pushImage(pX, pY, pW, pH, player);
  for (int i = 0; i < 10; i++) {
    tft.fillCircle(bx[i], by[i], bw + 2, TFT_BLACK);
    if (by[i] > 23) tft.fillCircle(bx[i], by[i], bw, TFT_RED);
  }

  for (int i = 0; i < 8; i++) {
    if (ex[i] < -32)
      tft.fillRect(0, 14, 32, 32, TFT_BLACK);
    else
      tft.pushImage(ex[i], ey[i], eh, ew, enemy[eFrame[i]]);
  }
}  // draw()
//-----------------------------------------------------------------------------
void player_move_left() {
  if (pX > 2) {
    pX = pX - pXs;
  }
}  // player_move_left()
//-----------------------------------------------------------------------------
void player_move_right() {
  if (pX < kScreenWidth - pW - 1) {
    pX = pX + pXs;
  }
}  // player_move_right()
//-----------------------------------------------------------------------------
void check_buttons() {
  if (only_left_button_pressed()) {
    player_move_left();
  }

  if (only_right_button_pressed()) {
    player_move_right();
  }

  if (fire_button_pressed()) {
    if (pres2 == 0) {
      if (fire == 0) {
        pres2 = 1;
        bx[counter] = pX + pW / 2;
        by[counter] = pY - bw / 2;
        counter++;
        fire = 1;
        if (counter >= 10) counter = 0;
      }
    }
  } else
    pres2 = 0;
}  // check_buttons()
//-----------------------------------------------------------------------------
void collision() {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 8; ++j) {
      if (pX + pW / 2 > ex[j] && pX + pW / 2 < ex[j] + ew && ey[j] + 26 > pY &&
          ey[j] < kScreenHeight - 10)
        phase = 2;

      if (bx[i] > ex[j] + 2 && bx[i] < (ex[j] + ew - 2) && by[i] < ey[j] + 20) {
        score++;
        tft.drawString(String(score), 48, 1, 2);
        by[i] = kScreenHeight + 10;
        el[j] = el[j] - 1;
        if (el[j] == 0) {
          score = score + 5;
          tft.fillRect(ex[j], ey[j], 32, 32, TFT_BLACK);
          ex[j] = pos[random(0, 4)];
          ey[j] = -50;
          eFrame[j] = random(0, 4);
          el[j] = 3;
        }
      }
    }
  }
}  // collision()
//-----------------------------------------------------------------------------
void play_game() {
  if (millis() - currentTime > 1000) {
    timeAlive++;
    tft.drawString(String(timeAlive), 105, 1, 2);

    currentTime = millis();
    if (timeAlive == newLevelTime) {
      esy = esy + 0.02;
      newLevelTime = timeAlive * 2;
    }
  }

  if (startCounter == 0)
    if (count < 8) {
      startCounter = 1;
      ey[count] = 19;
      count++;
      if (count == 8) count = 0;
      endCounter = random(180, 500);
    }

  draw();
  check_buttons();

  for (int i = 0; i < 10; ++i) {
    if (by[i] > 23 && by[i] < kScreenHeight) {
      by[i] = by[i] - bs;
    }
  }  // loop

  for (int i = 0; i < 8; i++) {
    if (ey[i] != -50.00) {
      ey[i] = ey[i] + esy;
      ex[i] = ex[i] - esx;
    }

    if (ey[i] > kScreenHeight + 2) {
      ex[i] = pos[random(0, 4)];
      ey[i] = -50;
      eFrame[i] = random(0, 4);
      esy = esy + 0.0025;
      el[i] = 3;
    }
  }

  if (fire > 0) {
    fire++;
  }
  if (fire > 60) {
    fire = 0;
  }

  if (startCounter > 0) {
    startCounter++;
  }

  if (startCounter >= endCounter) {
    startCounter = 0;
  }
  collision();
}  // play_game()
//-----------------------------------------------------------------------------
void restart() {
  esy = 0.11;
  for (int i = 0; i < 8; ++i) {
    ex[i] = pos[random(0, 4)];
    ey[i] = -50;
    eFrame[i] = random(0, 4);
    el[i] = 3;
  }

  for (int i = 0; i < 10; ++i) {
    by[i] = 250;
  }

  pres1 = 0;  // debounce left button
  pres2 = 0;  // debounce right button
  counter = 0;
  fire = 0;
  score = 0;
  timeAlive = 0;
  currentTime = 0;
  newLevelTime = 15;
  count = 0;
  startCounter = 0;
  endCounter = 0;
  aniFrame = 0;
}  // restart()
//-----------------------------------------------------------------------------
void game_over() {
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(0, 0, kScreenWidth, kScreenHeight, gameover);
  tft.drawString("SCORE: " + String(score), 25, 120, 2);
  tft.drawString("TIME: " + String(timeAlive), 25, 140, 2);
  delay(500);
  while (!fire_button_pressed()) {
    delay(10);
  }
  restart();
  phase = 0;
}  // game_over()
//-----------------------------------------------------------------------------
void loop() {
  if (phase == 0) {
    show_intro();
  }

  if (phase == 1) {
    play_game();
  }
  if (phase == 2) {
    game_over();
  }
}  // loop()
