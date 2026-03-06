// ==========================================
// GOLUBOT OS v9.0 - THE ULTIMATE MENU UPDATE
// ==========================================
#include <FS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define USE_WIFI false  // Keep false if running on laptop USB power

TFT_eSPI tft = TFT_eSPI(); 
WebServer server(80);

#define TOUCH_PIN 13

// --- MODES & MENU ---
enum AppMode { MODE_FACES, MODE_MENU, MODE_FEED, MODE_GAME, MODE_DASHBOARD, MODE_MOTIVATION, MODE_POMODORO, MODE_HACKER, MODE_MAGICBALL };
AppMode currentMode = MODE_FACES;

const int MENU_ITEMS = 8;
// Naye options Menu me add kar diye hain!
String menuOptions[MENU_ITEMS] = {
  "1. Pomodoro", 
  "2. Flappy Game", 
  "3. Feed Me", 
  "4. Magic 8-Ball", 
  "5. Dashboard", 
  "6. Anime Quotes", 
  "7. Hacker Mode", 
  "8. Exit"
};
int menuIndex = 0;

// --- EMOTIONS & NATKHAT AI ---
enum Emotion { NORMAL, HAPPY, SAD, LOOK_LEFT, LOOK_RIGHT, HEART, DANCE, PONG, SLEEPING, TETRIS };
Emotion currentEmotion = NORMAL;
String currentSpeech = "Hello!";
unsigned long speechClearTime = 0;
int golubotFullness = 50; 

// --- TOUCH UX (NON-BLOCKING MULTI-TAP) ---
unsigned long touchStartTime = 0;
unsigned long lastTapTime = 0;
int tapCount = 0;
int touchState = 0, lastTouchState = 0;
bool isHolding = false;
bool holdTriggered = false;
int lastDrawnTap = -1; // Tap Indicator ke liye

// --- FEATURE GLOBALS ---
// Game
float pY = 120, pVel = 0; float gravity = 0.35; int obsX = 240, score = 0; bool gameOver = false;
// Pomodoro
int pomoMin = 25, pomoSec = 0; bool pomoRunning = false; unsigned long lastPomoTick = 0;
// Feed
bool isFoodFalling = false; int foodY = 0;
// AI Animation
unsigned long lastTick = 0; int pongBallX = 120, pongDir = 5; int animFrame = 0;
// Hacker Mode
int hackerY[10]; 
// Auto-Tetris
int tetX = 110, tetY = 40; uint16_t tetColor = TFT_MAGENTA;
// Magic 8-Ball
String magicAnswers[6] = {"100% YES!", "No way bro.", "Maybe later...", "Ask again", "Definitely!", "Nah, forget it."};
String currentAnswer = "Tap to Ask!";

// --- ANIME QUOTES ---
String animeQuotes[5] = {
  "I never give up!\n\n- Naruto", 
  "Surpass your limits!\n\n- Yami", 
  "Power comes in response\nto a need.\n\n- Goku",
  "If you don't take risks,\nyou can't create a future!\n\n- Luffy",
  "Tatatakae!\n\n- Eren"
};
int quoteIndex = 0;

// --- COLORS ---
#define EYE_COLOR TFT_CYAN
#define BG_COLOR TFT_BLACK
#define FOOD_COLOR TFT_ORANGE 

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);
  tft.init(); tft.setRotation(0); tft.fillScreen(BG_COLOR);

  tft.setTextColor(EYE_COLOR); tft.setTextSize(2);
  tft.setCursor(40, 110); tft.print("Booting v9.0...");
  delay(1000); 

  for(int i=0; i<10; i++) hackerY[i] = random(-100, 0);

  tft.fillScreen(BG_COLOR);
  currentSpeech = "I'm Golubot!";
  speechClearTime = millis() + 3000;
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 🌟 NON-BLOCKING TOUCH ENGINE ---
  touchState = digitalRead(TOUCH_PIN);
  
  if (touchState == HIGH && lastTouchState == LOW) {
    touchStartTime = currentMillis;
    isHolding = true; holdTriggered = false;
  } 
  else if (touchState == LOW && lastTouchState == HIGH) {
    unsigned long holdDuration = currentMillis - touchStartTime;
    isHolding = false;
    tft.fillRect(70, 230, 100, 4, BG_COLOR); // Clear hold bar
    
    if (holdDuration < 300 && !holdTriggered) { 
      tapCount++;
      lastTapTime = currentMillis;
    }
  }

  // LONG PRESS LOGIC (Hold Bar)
  if (isHolding && !holdTriggered && (currentMillis - touchStartTime) > 200) {
    int barWidth = map(currentMillis - touchStartTime, 200, 700, 0, 100);
    tft.fillRect(70, 230, constrain(barWidth, 0, 100), 4, TFT_GREEN);
    
    if (currentMillis - touchStartTime > 700) { 
      holdTriggered = true; tapCount = 0; tft.fillScreen(BG_COLOR);
      
      if (currentMode == MODE_FACES) currentMode = MODE_MENU;
      else if (currentMode == MODE_MENU) {
        // Updated Master Menu Routing
        if(menuIndex == 0) { currentMode = MODE_POMODORO; pomoRunning=false; pomoMin=25; pomoSec=0; }
        else if(menuIndex == 1) { currentMode = MODE_GAME; score=0; gameOver=false; obsX=240; pY=120; }
        else if(menuIndex == 2) currentMode = MODE_FEED;
        else if(menuIndex == 3) { currentMode = MODE_MAGICBALL; currentAnswer = "Tap to Ask!"; }
        else if(menuIndex == 4) currentMode = MODE_DASHBOARD;
        else if(menuIndex == 5) { currentMode = MODE_MOTIVATION; quoteIndex = random(0, 5); }
        else if(menuIndex == 6) currentMode = MODE_HACKER;
        else if(menuIndex == 7) currentMode = MODE_FACES; // Exit
      } else {
        currentMode = MODE_FACES; // Universal Exit via Long Press
      }
    }
  }

  // MULTI-TAP LOGIC (Wait for 400ms to evaluate total taps)
  if (tapCount > 0 && (currentMillis - lastTapTime > 400) && !isHolding) {
    if (currentMode == MODE_FACES) {
      if (tapCount == 1) { currentEmotion = HAPPY; currentSpeech = "Hehehe!"; speechClearTime = currentMillis + 1500; tft.fillScreen(BG_COLOR); }
      // Dashboard, Anime, and Hacker are now easily accessible via Menu, but multi-tap still works as a shortcut!
      else if (tapCount == 2) { currentMode = MODE_DASHBOARD; tft.fillScreen(BG_COLOR); }
      else if (tapCount == 3) { currentMode = MODE_MOTIVATION; quoteIndex = random(0, 5); tft.fillScreen(BG_COLOR); }
      else if (tapCount >= 4) { currentMode = MODE_HACKER; tft.fillScreen(BG_COLOR); }
    }
    else if (currentMode == MODE_MENU && tapCount == 1) { menuIndex = (menuIndex + 1) % MENU_ITEMS; tft.fillScreen(BG_COLOR); }
    else if (currentMode == MODE_GAME && tapCount == 1) { if(gameOver) { score=0; obsX=240; pY=120; gameOver=false; tft.fillScreen(BG_COLOR); } else { pVel = -4.5; } }
    else if (currentMode == MODE_FEED && tapCount == 1) { if(!isFoodFalling) { isFoodFalling = true; foodY = 20; } }
    else if (currentMode == MODE_POMODORO && tapCount == 1) { pomoRunning = !pomoRunning; }
    else if (currentMode == MODE_MAGICBALL && tapCount == 1) { currentAnswer = magicAnswers[random(0,6)]; tft.fillScreen(BG_COLOR); }
    else if (currentMode != MODE_FACES && tapCount == 1) { currentMode = MODE_FACES; tft.fillScreen(BG_COLOR); } 
    
    tapCount = 0;
  }
  lastTouchState = touchState;

  // --- RENDER ROUTER ---
  if (currentMode == MODE_FACES) renderFaces(currentMillis);
  else if (currentMode == MODE_MENU) renderMenu();
  else if (currentMode == MODE_GAME) renderGame(currentMillis);
  else if (currentMode == MODE_FEED) renderFeed(currentMillis);
  else if (currentMode == MODE_DASHBOARD) renderDashboard(currentMillis);
  else if (currentMode == MODE_MOTIVATION) renderMotivation();
  else if (currentMode == MODE_POMODORO) renderPomodoro(currentMillis);
  else if (currentMode == MODE_HACKER) renderHackerMode(currentMillis);
  else if (currentMode == MODE_MAGICBALL) renderMagicBall(currentMillis);
}

// ==========================================
// 1. NATKHAT AI & FACES
// ==========================================
void renderFaces(unsigned long currentMillis) {
  // --- VISUAL TAP INDICATOR (NEW) ---
  if (tapCount > 0) {
    if (tapCount != lastDrawnTap) {
      tft.fillRect(170, 5, 70, 25, BG_COLOR); // Clear old tap text
      tft.setTextColor(TFT_YELLOW); tft.setTextSize(2);
      tft.setCursor(175, 10); tft.print("Tap:"); tft.print(tapCount);
      lastDrawnTap = tapCount;
    }
  } else if (lastDrawnTap != -1) {
    tft.fillRect(170, 5, 70, 25, BG_COLOR); // Clean up after timeout
    lastDrawnTap = -1;
  }

  if (currentSpeech != "" && currentMillis > speechClearTime) { currentSpeech = ""; tft.fillScreen(BG_COLOR); }
  
  if (currentMillis - lastTick > random(4000, 8000)) {
    int rnd = random(0, 100);
    if (rnd < 10) { currentEmotion = HEART; currentSpeech = "Love u bro!"; speechClearTime = currentMillis + 2000; }
    else if (rnd < 20) { currentEmotion = DANCE; currentSpeech = "Party Time!"; speechClearTime = currentMillis + 3000; animFrame = 0; }
    else if (rnd < 35) { currentEmotion = PONG; currentSpeech = "Playing Pong..."; speechClearTime = currentMillis + 4000; }
    else if (rnd < 50) { currentEmotion = TETRIS; currentSpeech = "Tetris mode!"; speechClearTime = currentMillis + 4000; tetY = 40; }
    else if (rnd < 60) { currentEmotion = SLEEPING; currentSpeech = "Zzz..."; speechClearTime = currentMillis + 3000; }
    else if (rnd < 70) { currentEmotion = LOOK_LEFT; }
    else if (rnd < 80) { currentEmotion = LOOK_RIGHT; }
    else { currentEmotion = NORMAL; animFrame = 1; } // Blink
    lastTick = currentMillis; tft.fillScreen(BG_COLOR);
  }

  if (currentEmotion != NORMAL && currentEmotion != DANCE && currentEmotion != PONG && currentEmotion != TETRIS && currentMillis - lastTick > 2500) {
    currentEmotion = NORMAL; tft.fillScreen(BG_COLOR);
  }

  if (currentSpeech != "") {
    tft.setTextSize(1); tft.setTextColor(TFT_YELLOW); tft.setCursor(40, 180); tft.print(currentSpeech);
  }

  // --- DRAW EMOTIONS ---
  if (currentEmotion == HEART) {
    tft.fillCircle(100, 110, 20, TFT_RED); tft.fillCircle(140, 110, 20, TFT_RED);
    tft.fillTriangle(80, 115, 160, 115, 120, 155, TFT_RED);
  } 
  else if (currentEmotion == DANCE) {
    if (currentMillis % 200 < 10) tft.fillScreen(BG_COLOR); 
    int yOffset = ((currentMillis / 200) % 2 == 0) ? -15 : 15;
    tft.fillRoundRect(60, 100 + yOffset, 30, 40, 10, TFT_MAGENTA);
    tft.fillRoundRect(150, 100 - yOffset, 30, 40, 10, TFT_GREEN);
  }
  else if (currentEmotion == PONG) {
    if (currentMillis % 30 < 10) {
      tft.fillCircle(pongBallX, 110, 8, BG_COLOR); 
      pongBallX += pongDir;
      if(pongBallX > 170 || pongBallX < 60) pongDir = -pongDir;
    }
    tft.fillRect(50, 90, 10, 40, EYE_COLOR); 
    tft.fillRect(180, 90, 10, 40, EYE_COLOR); 
    tft.fillCircle(pongBallX, 110, 8, TFT_WHITE);
  }
  else if (currentEmotion == TETRIS) { 
    if (currentMillis % 50 < 10) {
      tft.fillRect(tetX, tetY-5, 20, 20, BG_COLOR); 
      tetY += 5;
      if (random(0,10) > 7) {
        tft.fillRect(tetX, tetY, 20, 20, BG_COLOR); 
        tetX += (random(0,2)==0 ? -20 : 20); 
        tetX = constrain(tetX, 60, 160);
      }
      if (tetY > 160) { tetY = 40; tetX = 110; tetColor = random(0x1111, 0xFFFF); tft.fillScreen(BG_COLOR); }
      tft.fillRect(tetX, tetY, 20, 20, tetColor); 
    }
  }
  else if (currentEmotion == SLEEPING) {
    tft.fillRect(60, 120, 30, 5, TFT_DARKGREY); tft.fillRect(150, 120, 30, 5, TFT_DARKGREY);
  }
  else if (currentEmotion == HAPPY) {
    tft.fillRoundRect(60, 110, 30, 15, 5, EYE_COLOR); tft.fillRoundRect(150, 110, 30, 15, 5, EYE_COLOR);
    tft.fillTriangle(60, 125, 90, 125, 75, 115, BG_COLOR); tft.fillTriangle(150, 125, 180, 125, 165, 115, BG_COLOR);
  } 
  else {
    int lx = 60, rx = 150;
    if (currentEmotion == LOOK_LEFT) { lx -= 15; rx -= 15; } else if (currentEmotion == LOOK_RIGHT) { lx += 15; rx += 15; }
    
    if (animFrame == 1 && currentMillis - lastTick < 150) { 
      tft.fillRect(lx, 118, 30, 4, EYE_COLOR); tft.fillRect(rx, 118, 30, 4, EYE_COLOR);
    } else {
      animFrame = 0;
      tft.fillRoundRect(lx, 100, 30, 40, 10, EYE_COLOR); tft.fillRoundRect(rx, 100, 30, 40, 10, EYE_COLOR);
    }
  }
}

// ==========================================
// 2. MAGIC 8-BALL 
// ==========================================
void renderMagicBall(unsigned long currentMillis) {
  tft.drawCircle(120, 120, 100, TFT_PURPLE);
  tft.fillTriangle(120, 60, 70, 150, 170, 150, TFT_DARKGREY); 
  
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(65, 30); tft.print("8-BALL");
  
  tft.setTextSize(1); tft.setTextColor(TFT_YELLOW);
  if(currentAnswer == "Tap to Ask!") tft.setCursor(85, 110);
  else tft.setCursor(75, 110);
  tft.print(currentAnswer);
  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(60, 180); tft.print("Tap to Ask | Hold = Exit");
}

// ==========================================
// 3. POMODORO TIMER
// ==========================================
void renderPomodoro(unsigned long currentMillis) {
  if (pomoRunning && currentMillis - lastPomoTick >= 1000) {
    lastPomoTick = currentMillis;
    if (pomoSec == 0) {
      if (pomoMin == 0) pomoRunning = false;
      else { pomoMin--; pomoSec = 59; }
    } else { pomoSec--; }
    tft.fillRect(40, 80, 160, 60, BG_COLOR); 
  }
  tft.drawCircle(120, 120, 110, TFT_RED);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2); tft.setCursor(75, 60); tft.print("FOCUS");
  tft.setTextSize(4); tft.setCursor(65, 100);
  if(pomoMin < 10) tft.print("0"); tft.print(pomoMin); tft.print(":");
  if(pomoSec < 10) tft.print("0"); tft.print(pomoSec);
  tft.setTextSize(1); tft.setCursor(60, 160); tft.print(pomoRunning ? "Tap to Pause" : "Tap to Start");
}

// ==========================================
// 4. CYBER DASHBOARD
// ==========================================
void renderDashboard(unsigned long currentMillis) {
  tft.drawCircle(120, 120, 115, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN); tft.setTextSize(2); tft.setCursor(65, 40); tft.print("GOLUBOT");
  int m = (currentMillis / 60000) % 60; int s = (currentMillis / 1000) % 60;
  tft.setTextColor(TFT_WHITE); tft.setTextSize(4); tft.setCursor(65, 90);
  if(m < 10) tft.print("0"); tft.print(m); tft.print(":");
  if(s < 10) tft.print("0"); tft.print(s);
  tft.setTextSize(1); tft.setTextColor(TFT_GREEN); tft.setCursor(70, 145); tft.print("POWER: "); tft.print(golubotFullness); tft.print("%");
  tft.drawRect(50, 160, 140, 10, TFT_WHITE); tft.fillRect(52, 162, map(golubotFullness, 0, 100, 0, 136), 6, TFT_GREEN);
  tft.setCursor(60, 190); tft.setTextColor(TFT_DARKGREY); tft.print("Tap to Exit");
}

// ==========================================
// 5. ANIME MOTIVATION
// ==========================================
void renderMotivation() {
  tft.drawRect(20, 40, 200, 160, TFT_YELLOW);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(30, 80); tft.print(animeQuotes[quoteIndex]);
  tft.setTextSize(1); tft.setTextColor(TFT_DARKGREY); tft.setCursor(70, 180); tft.print("Tap to close");
}

// ==========================================
// 6. HACKER MODE (MATRIX)
// ==========================================
void renderHackerMode(unsigned long currentMillis) {
  if (currentMillis % 50 < 10) {
    for(int i=0; i<10; i++) {
      tft.setTextColor(TFT_BLACK); tft.setCursor(30 + (i*18), hackerY[i]-15); tft.print("0"); 
      hackerY[i] += 15;
      if(hackerY[i] > 240) hackerY[i] = random(-50, 0);
      tft.setTextColor(TFT_GREEN); tft.setCursor(30 + (i*18), hackerY[i]); 
      tft.print(random(0,2) == 0 ? "0" : "1");
    }
  }
}

// ==========================================
// 7. FEED & GAME 
// ==========================================
void renderFeed(unsigned long currentMillis) {
  tft.setTextSize(1); tft.setTextColor(TFT_WHITE); tft.setCursor(60, 40); tft.print("Fullness: "); tft.print(golubotFullness); tft.print("%");
  tft.setCursor(40, 190); tft.print("Tap to Drop Food!");
  tft.fillRoundRect(80, 140, 80, 20, 10, isFoodFalling && foodY > 100 ? TFT_RED : TFT_DARKGREY);
  tft.fillRoundRect(90, 100, 20, 20, 5, EYE_COLOR); tft.fillRoundRect(130, 100, 20, 20, 5, EYE_COLOR); 

  if (isFoodFalling) {
    if (currentMillis % 20 < 10) {
      tft.fillCircle(120, foodY - 5, 8, BG_COLOR); foodY += 6; tft.fillCircle(120, foodY, 8, FOOD_COLOR);
    }
    if (foodY > 140) {
      isFoodFalling = false; golubotFullness = min(100, golubotFullness + 15);
      tft.fillScreen(BG_COLOR); tft.setCursor(90, 180); tft.setTextColor(TFT_YELLOW); tft.print("Yummy!");
      delay(500); tft.fillScreen(BG_COLOR);
    }
  }
}

void renderGame(unsigned long currentMillis) {
  if (currentMillis - lastTick < 20) return; lastTick = currentMillis;
  if (gameOver) {
    tft.setCursor(50, 100); tft.setTextColor(TFT_RED); tft.setTextSize(3); tft.print("CRASHED!");
    tft.setCursor(70, 140); tft.setTextColor(TFT_WHITE); tft.setTextSize(2); tft.print("Score: "); tft.print(score);
    return;
  }
  tft.fillCircle(60, (int)pY, 12, BG_COLOR); tft.fillRect(obsX + 4, 140, 15, 32, BG_COLOR); 
  pVel += gravity; pY += pVel;
  if (pY > 160) { pY = 160; pVel = 0; } if (pY < 20) { pY = 20; pVel = 0; }
  obsX -= 4; if (obsX < 0) { obsX = 240; score++; }
  if (obsX < 72 && obsX > 48 && pY > 128) { gameOver = true; tft.fillScreen(BG_COLOR); return; }
  tft.drawLine(0, 172, 240, 172, TFT_GREEN); tft.setCursor(110, 30); tft.setTextColor(TFT_WHITE); tft.setTextSize(2); tft.print(score);
  tft.fillCircle(60, (int)pY, 10, EYE_COLOR); tft.fillRect(obsX, 140, 15, 32, TFT_RED);
}

void renderMenu() {
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2); tft.setCursor(90, 20); tft.print("MENU");
  for(int i=0; i<MENU_ITEMS; i++) {
    tft.setTextColor(i == menuIndex ? TFT_GREEN : TFT_DARKGREY);
    // Menu ko thoda tight kiya taaki saare 8 options screen par aaram se fit ho jayein
    tft.setCursor(20, 50 + (i*20)); 
    tft.print(menuOptions[i]);
  }
  tft.setTextSize(1); tft.setTextColor(TFT_WHITE); tft.setCursor(50, 215); tft.print("Hold green bar to Select");
}