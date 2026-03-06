// ==========================================
// GOLUBOT OS v10.0 - ADVANCED DESK PET AI
// ESP32 + GC9A01 Round Display + Capacitive Touch
// ==========================================
//
// SETUP: Configure TFT_eSPI for GC9A01 display.
//   Copy tft_setup_gc9a01.h settings into your
//   TFT_eSPI User_Setup.h file. See README.md.
// ==========================================

#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- HARDWARE CONFIG ---
// Touch: Uses ESP32 built-in capacitive touch on GPIO13 (T4)
// Set USE_CAPACITIVE_TOUCH to false for digital touch module (TTP223)
#define USE_CAPACITIVE_TOUCH true
#define TOUCH_PIN 13
#define TOUCH_THRESHOLD 40  // For capacitive mode (lower = more sensitive)

// --- DISPLAY (GC9A01 240x240 Round) ---
#define SCREEN_W 240
#define SCREEN_H 240
#define CX 120  // Center X
#define CY 120  // Center Y

// --- COLORS ---
#define BG_COLOR    TFT_BLACK
#define EYE_WHITE   0xFFFF
#define EYE_COLOR   TFT_CYAN
#define PUPIL_COLOR 0x0000
#define MOUTH_COLOR TFT_WHITE
#define BLUSH_COLOR 0xFB56  // Pinkish
#define RING_COLOR  0x2945  // Dark blue-grey
#define RING_HAPPY  0x0410  // Green tint when happy
#define RING_TIRED  0x4000  // Red tint when tired

// --- MODES ---
enum AppMode {
  MODE_FACES, MODE_MENU, MODE_FEED, MODE_GAME,
  MODE_DASHBOARD, MODE_MOTIVATION, MODE_POMODORO,
  MODE_HACKER, MODE_MAGICBALL
};
AppMode currentMode = MODE_FACES;

// --- MENU ---
const int MENU_ITEMS = 8;
String menuOptions[MENU_ITEMS] = {
  "Pomodoro", "Flappy", "Feed Me",
  "8-Ball", "Dashboard", "Quotes",
  "Matrix", "Exit"
};
int menuIndex = 0;

// --- EMOTIONS (Enhanced for EMO-style) ---
enum Emotion {
  NORMAL, HAPPY, SAD, LOOK_LEFT, LOOK_RIGHT,
  HEART, DANCE, SLEEPING, EXCITED, ANGRY,
  CONFUSED, SHY, CURIOUS, BORED
};
Emotion currentEmotion = NORMAL;
Emotion prevDrawnEmotion = NORMAL;

// --- MOOD SYSTEM ---
int moodEnergy = 80;      // 0-100: Decreases over time, increases with feeding
int moodHappiness = 70;   // 0-100: Increases with interaction, slowly decreases
unsigned long lastMoodTick = 0;

// --- FACE ANIMATION ---
// Pupils (smooth movement)
float leftPupilX = 0, leftPupilY = 0;
float rightPupilX = 0, rightPupilY = 0;
float targetPupilX = 0, targetPupilY = 0;
unsigned long lastPupilChange = 0;
unsigned long nextPupilChangeTime = 0;
int prevLPX = 80, prevLPY = 105;
int prevRPX = 160, prevRPY = 105;

// Blink (natural cycle)
float eyeOpenness = 1.0;
enum BlinkState { BLINK_OPEN, BLINK_CLOSING, BLINK_CLOSED, BLINK_OPENING };
BlinkState blinkState = BLINK_OPEN;
unsigned long nextBlinkTime = 0;
unsigned long blinkStartTime = 0;

// Speech
String currentSpeech = "";
unsigned long speechClearTime = 0;
int golubotFullness = 50;

// --- TOUCH ---
unsigned long touchStartTime = 0;
unsigned long lastTapTime = 0;
int tapCount = 0;
bool isTouched = false;
bool wasTouched = false;
bool isHolding = false;
bool holdTriggered = false;

// --- FEATURE GLOBALS ---
// Game
float pY = 120, pVel = 0;
float grav = 0.35;
int obsX = 240, score = 0;
bool gameOver = false;
// Pomodoro
int pomoMin = 25, pomoSec = 0;
bool pomoRunning = false;
unsigned long lastPomoTick = 0;
// Feed
bool isFoodFalling = false;
int foodY = 0;
// Animation
unsigned long lastTick = 0;
unsigned long lastFaceRender = 0;
int animFrame = 0;
bool faceNeedsRedraw = true;
// Hacker
int hackerY[10];
// Magic 8-Ball
String magicAnswers[8] = {
  "100% YES!", "No way!", "Maybe...", "Ask again",
  "Definitely!", "Nah.", "Try later", "Of course!"
};
String currentAnswer = "Tap to Ask!";
// Quotes
String animeQuotes[7] = {
  "I never give up!\n- Naruto",
  "Surpass your limits!\n- Yami",
  "Power comes from\nneed. - Goku",
  "Take risks to\ncreate a future!\n- Luffy",
  "Tatakae!\n- Eren",
  "The world isn't\nperfect but it's\nthere for us.\n- Roy Mustang",
  "Believe in yourself!\n- Gurren Lagann"
};
int quoteIndex = 0;

// --- UTILITY ---
float lerpf(float a, float b, float t) {
  return a + (b - a) * constrain(t, 0.0f, 1.0f);
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  Serial.println("GOLUBOT OS v10.0 - Advanced Desk Pet");

#if !USE_CAPACITIVE_TOUCH
  pinMode(TOUCH_PIN, INPUT);
#endif

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(BG_COLOR);

  // Boot animation - expanding ring for round display
  for (int r = 0; r <= 119; r += 3) {
    tft.drawCircle(CX, CY, r, EYE_COLOR);
    delay(15);
  }
  tft.fillScreen(BG_COLOR);

  tft.setTextColor(EYE_COLOR);
  tft.setTextSize(2);
  tft.setCursor(30, 95);
  tft.print("GOLUBOT v10");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(40, 125);
  tft.print("Advanced Desk Pet AI");
  delay(1500);
  tft.fillScreen(BG_COLOR);

  for (int i = 0; i < 10; i++) hackerY[i] = random(-100, 0);
  nextBlinkTime = millis() + random(2000, 5000);
  lastMoodTick = millis();
  lastTick = millis();
  currentSpeech = "Hello! I'm Golubot!";
  speechClearTime = millis() + 3000;
}

// ==========================================
// TOUCH READING
// ==========================================
bool readTouch() {
#if USE_CAPACITIVE_TOUCH
  return touchRead(TOUCH_PIN) < TOUCH_THRESHOLD;
#else
  return digitalRead(TOUCH_PIN) == HIGH;
#endif
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  unsigned long now = millis();

  // --- TOUCH ENGINE ---
  isTouched = readTouch();

  if (isTouched && !wasTouched) {
    touchStartTime = now;
    isHolding = true;
    holdTriggered = false;
  }
  else if (!isTouched && wasTouched) {
    unsigned long holdDuration = now - touchStartTime;
    isHolding = false;
    tft.fillRoundRect(CX - 40, 218, 80, 5, 2, BG_COLOR);

    if (holdDuration < 300 && !holdTriggered) {
      tapCount++;
      lastTapTime = now;
      moodHappiness = min(100, moodHappiness + 2);
    }
  }

  // Long press with visual hold bar
  if (isHolding && !holdTriggered && (now - touchStartTime) > 200) {
    int barW = map(now - touchStartTime, 200, 700, 0, 80);
    barW = constrain(barW, 0, 80);
    tft.fillRoundRect(CX - 40, 218, barW, 5, 2, TFT_GREEN);

    if (now - touchStartTime > 700) {
      holdTriggered = true;
      tapCount = 0;
      tft.fillScreen(BG_COLOR);
      faceNeedsRedraw = true;

      if (currentMode == MODE_FACES) {
        currentMode = MODE_MENU;
      }
      else if (currentMode == MODE_MENU) {
        switch (menuIndex) {
          case 0: currentMode = MODE_POMODORO; pomoRunning = false; pomoMin = 25; pomoSec = 0; break;
          case 1: currentMode = MODE_GAME; score = 0; gameOver = false; obsX = 240; pY = 120; break;
          case 2: currentMode = MODE_FEED; break;
          case 3: currentMode = MODE_MAGICBALL; currentAnswer = "Tap to Ask!"; break;
          case 4: currentMode = MODE_DASHBOARD; break;
          case 5: currentMode = MODE_MOTIVATION; quoteIndex = random(0, 7); break;
          case 6: currentMode = MODE_HACKER; break;
          case 7: currentMode = MODE_FACES; break;
        }
      }
      else {
        currentMode = MODE_FACES;
      }
    }
  }

  // Multi-tap evaluation (wait 400ms for tap sequence to complete)
  if (tapCount > 0 && (now - lastTapTime > 400) && !isHolding) {
    handleTaps(tapCount, now);
    tapCount = 0;
  }
  wasTouched = isTouched;

  // --- MOOD UPDATE (every 60 seconds) ---
  if (now - lastMoodTick > 60000) {
    lastMoodTick = now;
    moodEnergy = max(0, moodEnergy - 1);
    moodHappiness = max(0, moodHappiness - 1);
    if (moodEnergy < 20) golubotFullness = max(0, golubotFullness - 2);
  }

  // --- RENDER ROUTER ---
  switch (currentMode) {
    case MODE_FACES:      renderFaces(now); break;
    case MODE_MENU:       renderMenu(); break;
    case MODE_GAME:       renderGame(now); break;
    case MODE_FEED:       renderFeed(now); break;
    case MODE_DASHBOARD:  renderDashboard(now); break;
    case MODE_MOTIVATION: renderMotivation(); break;
    case MODE_POMODORO:   renderPomodoro(now); break;
    case MODE_HACKER:     renderHackerMode(now); break;
    case MODE_MAGICBALL:  renderMagicBall(now); break;
  }
}

// ==========================================
// TAP HANDLER
// ==========================================
void handleTaps(int taps, unsigned long now) {
  switch (currentMode) {
    case MODE_FACES:
      faceNeedsRedraw = true;
      if (taps == 1) {
        currentEmotion = HAPPY;
        currentSpeech = "Hehehe!";
        speechClearTime = now + 1500;
      }
      else if (taps == 2) {
        currentEmotion = EXCITED;
        currentSpeech = "Woohoo!";
        speechClearTime = now + 1500;
      }
      else if (taps == 3) {
        currentMode = MODE_DASHBOARD;
        tft.fillScreen(BG_COLOR);
      }
      else if (taps >= 4) {
        currentMode = MODE_HACKER;
        tft.fillScreen(BG_COLOR);
      }
      break;

    case MODE_MENU:
      if (taps == 1) {
        menuIndex = (menuIndex + 1) % MENU_ITEMS;
        tft.fillScreen(BG_COLOR);
      }
      break;

    case MODE_GAME:
      if (taps == 1) {
        if (gameOver) {
          score = 0; obsX = 240; pY = 120;
          gameOver = false; tft.fillScreen(BG_COLOR);
        } else {
          pVel = -4.5;
        }
      }
      break;

    case MODE_FEED:
      if (taps == 1 && !isFoodFalling) {
        isFoodFalling = true;
        foodY = 30;
      }
      break;

    case MODE_POMODORO:
      if (taps == 1) pomoRunning = !pomoRunning;
      break;

    case MODE_MAGICBALL:
      if (taps == 1) {
        currentAnswer = magicAnswers[random(0, 8)];
        tft.fillScreen(BG_COLOR);
      }
      break;

    default:
      if (taps == 1) {
        currentMode = MODE_FACES;
        tft.fillScreen(BG_COLOR);
        faceNeedsRedraw = true;
      }
      break;
  }
}

// ==========================================
// FACE ANIMATION SYSTEM
// ==========================================

void updateBlink(unsigned long now) {
  switch (blinkState) {
    case BLINK_OPEN:
      eyeOpenness = 1.0;
      if (now > nextBlinkTime) {
        blinkState = BLINK_CLOSING;
        blinkStartTime = now;
      }
      break;
    case BLINK_CLOSING:
      eyeOpenness = 1.0 - (float)(now - blinkStartTime) / 80.0;
      if (eyeOpenness <= 0.0) {
        eyeOpenness = 0.0;
        blinkState = BLINK_CLOSED;
        blinkStartTime = now;
      }
      break;
    case BLINK_CLOSED:
      eyeOpenness = 0.0;
      if (now - blinkStartTime > 60) {
        blinkState = BLINK_OPENING;
        blinkStartTime = now;
      }
      break;
    case BLINK_OPENING:
      eyeOpenness = (float)(now - blinkStartTime) / 80.0;
      if (eyeOpenness >= 1.0) {
        eyeOpenness = 1.0;
        blinkState = BLINK_OPEN;
        nextBlinkTime = now + random(2000, 6000);
      }
      break;
  }
}

void updatePupils(unsigned long now) {
  if (now > nextPupilChangeTime) {
    nextPupilChangeTime = now + random(2000, 5000);
    targetPupilX = random(-8, 9);
    targetPupilY = random(-5, 6);
    if (moodEnergy < 20) targetPupilY = 3;  // Droopy when tired
  }
  leftPupilX = lerpf(leftPupilX, targetPupilX, 0.1);
  leftPupilY = lerpf(leftPupilY, targetPupilY, 0.1);
  rightPupilX = lerpf(rightPupilX, targetPupilX, 0.1);
  rightPupilY = lerpf(rightPupilY, targetPupilY, 0.1);
}

void drawEye(int cx, int cy, float offX, float offY, float openness, bool isAngry) {
  int eyeW = 26;
  int eyeH = (int)(20 * openness);

  if (eyeH < 2) {
    // Closed eye - just a line
    tft.fillRect(cx - eyeW, cy - 1, eyeW * 2, 3, EYE_COLOR);
    return;
  }

  // Eye white oval
  tft.fillEllipse(cx, cy, eyeW, eyeH, EYE_WHITE);

  // Pupil with highlight
  int px = cx + (int)offX;
  int py = cy + (int)offY;
  tft.fillCircle(px, py, 8, PUPIL_COLOR);
  tft.fillCircle(px + 3, py - 3, 2, EYE_WHITE);

  // Angry eyelid overlay
  if (isAngry && openness > 0.3) {
    tft.fillRect(cx - eyeW - 1, cy - eyeH - 1, eyeW * 2 + 2, 8, BG_COLOR);
  }
}

void drawMouth(Emotion emotion) {
  int mx = CX;
  int my = 155;

  switch (emotion) {
    case HAPPY:
    case EXCITED:
      // Wide smile arc
      for (int i = -20; i <= 20; i++) {
        int y = my + (i * i) / 30;
        tft.fillCircle(mx + i, y, 2, MOUTH_COLOR);
      }
      break;
    case SAD:
      // Frown arc
      for (int i = -15; i <= 15; i++) {
        int y = my + 8 - (i * i) / 25;
        tft.fillCircle(mx + i, y, 1, MOUTH_COLOR);
      }
      break;
    case ANGRY:
      // Gritted teeth
      tft.fillRect(mx - 15, my, 30, 6, MOUTH_COLOR);
      for (int i = 0; i < 5; i++) {
        tft.drawLine(mx - 15 + i * 8, my, mx - 15 + i * 8, my + 6, BG_COLOR);
      }
      break;
    case HEART:
      // Kiss
      tft.fillCircle(mx, my + 3, 5, TFT_RED);
      break;
    case SLEEPING:
      // Open mouth (snoring)
      tft.fillCircle(mx, my + 2, 7, MOUTH_COLOR);
      tft.fillCircle(mx, my, 4, BG_COLOR);
      break;
    case CONFUSED:
      // Wavy line using sine wave
      for (int i = -15; i <= 15; i++) {
        int y = my + (int)(3.0 * sin((float)i * 0.4));
        tft.fillCircle(mx + i, y, 1, MOUTH_COLOR);
      }
      break;
    case SHY:
      // Small offset mouth
      tft.fillRect(mx + 5, my, 10, 3, MOUTH_COLOR);
      break;
    case CURIOUS:
      // O shape
      tft.drawCircle(mx, my + 3, 7, MOUTH_COLOR);
      tft.drawCircle(mx, my + 3, 6, MOUTH_COLOR);
      break;
    case BORED:
      // Flat line
      tft.fillRect(mx - 12, my, 24, 3, TFT_DARKGREY);
      break;
    default:
      // Normal - gentle smile
      for (int i = -12; i <= 12; i++) {
        int y = my + (i * i) / 50;
        tft.fillRect(mx + i, y, 2, 2, MOUTH_COLOR);
      }
      break;
  }
}

void drawCheeks(Emotion emotion) {
  if (emotion == HAPPY || emotion == SHY || emotion == EXCITED) {
    tft.fillCircle(48, 138, 8, BLUSH_COLOR);
    tft.fillCircle(192, 138, 8, BLUSH_COLOR);
  }
}

// ==========================================
// FACE RENDERING (EMO-style Desk Pet)
// ==========================================
void renderFaces(unsigned long now) {
  updateBlink(now);
  updatePupils(now);

  // Frame limit ~25fps
  if (now - lastFaceRender < 40 && !faceNeedsRedraw) return;
  lastFaceRender = now;

  // Clear speech after timeout
  if (currentSpeech.length() > 0 && now > speechClearTime) {
    currentSpeech = "";
    faceNeedsRedraw = true;
  }

  // AI behavior - change emotion periodically
  if (now - lastTick > (unsigned long)random(4000, 8000)) {
    lastTick = now;
    faceNeedsRedraw = true;

    // Mood affects which emotions are chosen
    if (moodEnergy < 20) {
      currentEmotion = SLEEPING;
      currentSpeech = "Zzz...";
      speechClearTime = now + 3000;
    }
    else if (moodHappiness < 20) {
      currentEmotion = (random(0, 2) == 0) ? SAD : BORED;
      currentSpeech = (currentEmotion == SAD) ? "I'm lonely..." : "...";
      speechClearTime = now + 2000;
    }
    else {
      int rnd = random(0, 100);
      if      (rnd < 8)  { currentEmotion = HEART;      currentSpeech = "Love you!";   speechClearTime = now + 2000; }
      else if (rnd < 16) { currentEmotion = DANCE;      currentSpeech = "Party!";      speechClearTime = now + 3000; }
      else if (rnd < 24) { currentEmotion = EXCITED;    currentSpeech = "Wow!";        speechClearTime = now + 2000; }
      else if (rnd < 32) { currentEmotion = CURIOUS;    currentSpeech = "Hmm?";        speechClearTime = now + 2000; }
      else if (rnd < 38) { currentEmotion = SHY;        currentSpeech = "Umm...";      speechClearTime = now + 2000; }
      else if (rnd < 44) { currentEmotion = SLEEPING;   currentSpeech = "Zzz...";      speechClearTime = now + 3000; }
      else if (rnd < 54) { currentEmotion = LOOK_LEFT;  currentSpeech = ""; }
      else if (rnd < 64) { currentEmotion = LOOK_RIGHT; currentSpeech = ""; }
      else if (rnd < 70) { currentEmotion = CONFUSED;   currentSpeech = "What?";       speechClearTime = now + 2000; }
      else               { currentEmotion = NORMAL;     currentSpeech = ""; }
    }
  }

  // Auto-return from expressive emotions after 3 seconds
  if (currentEmotion != NORMAL && currentEmotion != SLEEPING &&
      currentEmotion != DANCE && now - lastTick > 3000) {
    currentEmotion = NORMAL;
    faceNeedsRedraw = true;
  }

  // Full redraw on emotion change
  if (faceNeedsRedraw || currentEmotion != prevDrawnEmotion) {
    tft.fillScreen(BG_COLOR);

    // Edge ring - color reflects mood
    uint16_t rc = RING_COLOR;
    if (moodHappiness > 70) rc = RING_HAPPY;
    else if (moodEnergy < 30) rc = RING_TIRED;
    tft.drawCircle(CX, CY, 118, rc);
    tft.drawCircle(CX, CY, 119, rc);

    prevDrawnEmotion = currentEmotion;
    faceNeedsRedraw = false;
  } else {
    // Partial clear: erase previous pupil positions for smooth animation
    tft.fillCircle(prevLPX, prevLPY, 10, EYE_WHITE);
    tft.fillCircle(prevRPX, prevRPY, 10, EYE_WHITE);
  }

  // Compute pupil offsets based on emotion
  float lpx = leftPupilX, lpy = leftPupilY;
  float rpx = rightPupilX, rpy = rightPupilY;
  float openL = eyeOpenness, openR = eyeOpenness;
  bool angry = false;

  switch (currentEmotion) {
    case LOOK_LEFT:  lpx = -10; rpx = -10; break;
    case LOOK_RIGHT: lpx = 10;  rpx = 10;  break;
    case SLEEPING:   openL = 0.0; openR = 0.0; break;
    case HAPPY:
    case EXCITED:    openL = max(eyeOpenness, 0.7f); openR = openL; break;
    case SAD:        lpy = 3; rpy = 3; break;
    case ANGRY:      angry = true; lpx = 0; rpx = 0; break;
    case SHY:        lpx = 8; rpx = -8; break;
    case CURIOUS:    lpy = -3; rpy = -3; break;
    default: break;
  }

  // Draw eyes with smooth pupils
  drawEye(80, 105, lpx, lpy, openL, angry);
  drawEye(160, 105, rpx, rpy, openR, angry);
  prevLPX = 80 + (int)lpx;
  prevLPY = 105 + (int)lpy;
  prevRPX = 160 + (int)rpx;
  prevRPY = 105 + (int)rpy;

  // Mouth expression
  drawMouth(currentEmotion);

  // Blush cheeks
  drawCheeks(currentEmotion);

  // Heart floating animation
  if (currentEmotion == HEART) {
    int bounce = (now / 300) % 2 == 0 ? 0 : -4;
    tft.fillCircle(CX - 10, 55 + bounce, 8, TFT_RED);
    tft.fillCircle(CX + 10, 55 + bounce, 8, TFT_RED);
    tft.fillTriangle(CX - 18, 58 + bounce, CX + 18, 58 + bounce, CX, 72 + bounce, TFT_RED);
  }

  // Sleeping Zzz bubbles
  if (currentEmotion == SLEEPING) {
    int f = (now / 500) % 3;
    tft.setTextColor(TFT_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(165 + f * 4, 85 - f * 8);
    tft.print("z");
    if (f > 0) {
      tft.setTextSize(2);
      tft.setCursor(170 + f * 4, 72 - f * 8);
      tft.print("Z");
    }
  }

  // Speech bubble
  if (currentSpeech.length() > 0) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW);
    int tw = currentSpeech.length() * 6;
    tft.setCursor(CX - tw / 2, 185);
    tft.print(currentSpeech);
  }

  // Mood indicator dots (bottom of round display)
  uint16_t ec = moodEnergy > 50 ? TFT_GREEN : (moodEnergy > 20 ? TFT_YELLOW : TFT_RED);
  uint16_t hc = moodHappiness > 50 ? TFT_CYAN : (moodHappiness > 20 ? TFT_YELLOW : TFT_MAGENTA);
  tft.fillCircle(CX - 10, 205, 3, ec);
  tft.fillCircle(CX + 10, 205, 3, hc);
}

// ==========================================
// MENU (Round Display Carousel)
// ==========================================
void renderMenu() {
  tft.setTextColor(EYE_COLOR);
  tft.setTextSize(2);
  tft.setCursor(80, 18);
  tft.print("MENU");

  // Show 5 visible items centered on selection
  int startIdx = menuIndex - 2;
  for (int i = 0; i < 5; i++) {
    int idx = (startIdx + i + MENU_ITEMS) % MENU_ITEMS;
    bool selected = (idx == menuIndex);
    tft.setTextSize(selected ? 2 : 1);
    tft.setTextColor(selected ? TFT_GREEN : TFT_DARKGREY);
    int yPos = 55 + i * 30;
    int xPos = selected ? 30 : 50;
    tft.setCursor(xPos, yPos);
    if (selected) tft.print("> ");
    tft.print(menuOptions[idx]);
  }

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(35, 210);
  tft.print("Tap=Next  Hold=Select");
}

// ==========================================
// MAGIC 8-BALL
// ==========================================
void renderMagicBall(unsigned long now) {
  tft.drawCircle(CX, CY, 100, TFT_PURPLE);
  tft.drawCircle(CX, CY, 99, TFT_PURPLE);
  tft.fillTriangle(CX, 60, 70, 150, 170, 150, TFT_DARKGREY);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(65, 30);
  tft.print("8-BALL");

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  int tw = currentAnswer.length() * 6;
  tft.setCursor(CX - tw / 2, 110);
  tft.print(currentAnswer);

  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(45, 190);
  tft.print("Tap=Ask | Hold=Exit");
}

// ==========================================
// POMODORO TIMER
// ==========================================
void renderPomodoro(unsigned long now) {
  if (pomoRunning && now - lastPomoTick >= 1000) {
    lastPomoTick = now;
    if (pomoSec == 0) {
      if (pomoMin == 0) pomoRunning = false;
      else { pomoMin--; pomoSec = 59; }
    } else { pomoSec--; }
    tft.fillRect(40, 80, 160, 60, BG_COLOR);
  }

  tft.drawCircle(CX, CY, 110, TFT_RED);
  tft.drawCircle(CX, CY, 109, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(75, 60);
  tft.print("FOCUS");

  tft.setTextSize(4);
  tft.setCursor(55, 100);
  if (pomoMin < 10) tft.print("0");
  tft.print(pomoMin);
  tft.print(":");
  if (pomoSec < 10) tft.print("0");
  tft.print(pomoSec);

  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(55, 165);
  tft.print(pomoRunning ? "Tap=Pause | Hold=Exit" : "Tap=Start | Hold=Exit");
}

// ==========================================
// DASHBOARD (with Mood Stats)
// ==========================================
void renderDashboard(unsigned long now) {
  tft.drawCircle(CX, CY, 115, TFT_DARKGREY);

  tft.setTextColor(EYE_COLOR);
  tft.setTextSize(2);
  tft.setCursor(50, 35);
  tft.print("GOLUBOT");

  int m = (now / 60000) % 60;
  int s = (now / 1000) % 60;
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 70);
  if (m < 10) tft.print("0");
  tft.print(m);
  tft.print(":");
  if (s < 10) tft.print("0");
  tft.print(s);

  // Energy bar
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(50, 110);
  tft.print("Energy: ");
  tft.print(moodEnergy);
  tft.print("%");
  tft.drawRect(50, 122, 140, 8, TFT_WHITE);
  tft.fillRect(52, 124, map(moodEnergy, 0, 100, 0, 136), 4, TFT_GREEN);

  // Happiness bar
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 136);
  tft.print("Happy: ");
  tft.print(moodHappiness);
  tft.print("%");
  tft.drawRect(50, 148, 140, 8, TFT_WHITE);
  tft.fillRect(52, 150, map(moodHappiness, 0, 100, 0, 136), 4, TFT_CYAN);

  // Fullness bar
  tft.setTextColor(TFT_ORANGE);
  tft.setCursor(50, 162);
  tft.print("Full: ");
  tft.print(golubotFullness);
  tft.print("%");
  tft.drawRect(50, 174, 140, 8, TFT_WHITE);
  tft.fillRect(52, 176, map(golubotFullness, 0, 100, 0, 136), 4, TFT_ORANGE);

  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(70, 195);
  tft.print("Hold to Exit");
}

// ==========================================
// ANIME MOTIVATION
// ==========================================
void renderMotivation() {
  tft.drawCircle(CX, CY, 100, TFT_YELLOW);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(35, 85);
  tft.print(animeQuotes[quoteIndex]);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(60, 190);
  tft.print("Tap to close");
}

// ==========================================
// HACKER MODE (MATRIX)
// ==========================================
void renderHackerMode(unsigned long now) {
  if (now % 50 < 10) {
    for (int i = 0; i < 10; i++) {
      tft.setTextColor(BG_COLOR);
      tft.setCursor(30 + (i * 18), hackerY[i] - 15);
      tft.print("0");
      hackerY[i] += 15;
      if (hackerY[i] > 240) hackerY[i] = random(-50, 0);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(30 + (i * 18), hackerY[i]);
      tft.print(random(0, 2) == 0 ? "0" : "1");
    }
  }
}

// ==========================================
// FEED (with Mood Integration)
// ==========================================
void renderFeed(unsigned long now) {
  tft.drawCircle(CX, CY, 110, TFT_DARKGREY);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(55, 40);
  tft.print("Fullness: ");
  tft.print(golubotFullness);
  tft.print("%");

  // Draw pet face
  tft.fillRoundRect(85, 100, 20, 20, 5, EYE_COLOR);
  tft.fillRoundRect(135, 100, 20, 20, 5, EYE_COLOR);
  tft.fillRoundRect(80, 140, 80, 15, 8, isFoodFalling && foodY > 100 ? TFT_RED : TFT_DARKGREY);

  tft.setCursor(40, 195);
  tft.print("Tap=Feed | Hold=Exit");

  if (isFoodFalling) {
    if (now % 20 < 10) {
      tft.fillCircle(CX, foodY - 5, 8, BG_COLOR);
      foodY += 6;
      tft.fillCircle(CX, foodY, 8, TFT_ORANGE);
    }
    if (foodY > 140) {
      isFoodFalling = false;
      golubotFullness = min(100, golubotFullness + 15);
      moodEnergy = min(100, moodEnergy + 5);
      moodHappiness = min(100, moodHappiness + 3);
      tft.fillScreen(BG_COLOR);
      tft.setTextColor(TFT_YELLOW);
      tft.setTextSize(2);
      tft.setCursor(75, CY);
      tft.print("Yummy!");
      delay(500);
      tft.fillScreen(BG_COLOR);
    }
  }
}

// ==========================================
// FLAPPY GAME
// ==========================================
void renderGame(unsigned long now) {
  if (now - lastTick < 20) return;
  lastTick = now;

  if (gameOver) {
    tft.setTextColor(TFT_RED);
    tft.setTextSize(3);
    tft.setCursor(35, 90);
    tft.print("CRASHED!");
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(60, 130);
    tft.print("Score: ");
    tft.print(score);
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(55, 170);
    tft.print("Tap=Retry | Hold=Exit");
    return;
  }

  tft.fillCircle(60, (int)pY, 12, BG_COLOR);
  tft.fillRect(obsX + 4, 140, 15, 32, BG_COLOR);

  pVel += grav;
  pY += pVel;
  if (pY > 160) { pY = 160; pVel = 0; }
  if (pY < 20) { pY = 20; pVel = 0; }

  obsX -= 4;
  if (obsX < 0) { obsX = 240; score++; }

  if (obsX < 72 && obsX > 48 && pY > 128) {
    gameOver = true;
    tft.fillScreen(BG_COLOR);
    return;
  }

  tft.drawLine(0, 172, 240, 172, TFT_GREEN);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(CX - 5, 25);
  tft.print(score);
  tft.fillCircle(60, (int)pY, 10, EYE_COLOR);
  tft.fillRect(obsX, 140, 15, 32, TFT_RED);
}
