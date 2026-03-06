# GOLUBOT OS v10.0 - Advanced Desk Pet AI

An interactive, EMO-inspired desk pet firmware for ESP32 with a round GC9A01 display and capacitive touch sensor.

## Features

### Lifelike AI Pet
- **Smooth eye animations** with moving pupils and natural blinking
- **15 emotions**: Normal, Happy, Sad, Excited, Angry, Confused, Shy, Curious, Bored, Heart, Dance, Sleeping, Singing, Look Left, Look Right
- **Singing animation** with floating music notes
- **Mood system**: Energy and happiness levels that change over time and affect behavior
- **Speech bubbles** with contextual messages
- **Blush cheeks** for happy and shy emotions
- **Mood-colored edge ring** around the round display

### Interactive Modes
- **Pomodoro Timer** — 25-minute focus sessions with start/pause
- **Flappy Game** — Tap-to-fly obstacle avoidance with score tracking
- **Feed Me** — Drop food to keep your pet happy and boost energy
- **Magic 8-Ball** — Ask questions, get random answers
- **Dashboard** — View uptime, energy, happiness, and fullness stats
- **Anime Quotes** — Motivational quotes from popular anime
- **Matrix Mode** — Falling binary hacker animation
- **Chat Mode** — Text communication with Golubot (tap to cycle through messages and replies)
- **Anime Faces** — Pixel art character display featuring Naruto (Sage Mode), Gojo Satoru, and Luffy

### Controls

| Action | Effect |
|--------|--------|
| Single Tap | Interact / Navigate |
| Double Tap | Excited reaction |
| Triple Tap | Dashboard shortcut |
| 4+ Taps | Matrix mode shortcut |
| Long Hold | Open menu / Select / Exit |

## Hardware Required

| Component | Description |
|-----------|-------------|
| ESP32 Dev Kit | ESP32-WROOM-32 development board |
| GC9A01 Display | 1.28" Round TFT 240x240 SPI |
| Touch Sensor | ESP32 built-in capacitive touch (GPIO13) or TTP223 module |

## Wiring

### GC9A01 Display → ESP32

| GC9A01 Pin | ESP32 Pin | Description |
|------------|-----------|-------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO18 | SPI Clock |
| SDA | GPIO23 | SPI Data (MOSI) |
| CS | GPIO5 | Chip Select |
| DC | GPIO16 | Data/Command |
| RST | GPIO17 | Reset |
| BL | GPIO4 | Backlight (optional) |

### Touch Sensor

- **Built-in capacitive touch**: Connect a touch pad or wire to **GPIO13** (ESP32 touch pad T4)
- **TTP223 module**: Connect signal pin to **GPIO13**, VCC to 3.3V, GND to GND

## Arduino IDE Setup

### 1. Install Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add to Additional Boards Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for **esp32** and install

### 2. Install TFT_eSPI Library

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for **TFT_eSPI** by Bodmer
3. Click Install

### 3. Configure TFT_eSPI for GC9A01

1. Find your Arduino libraries folder:
   - **Windows**: `Documents\Arduino\libraries\TFT_eSPI\`
   - **Mac**: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - **Linux**: `~/Arduino/libraries/TFT_eSPI/`
2. Open `User_Setup.h`
3. Comment out all existing driver definitions
4. Copy the settings from [`tft_setup_gc9a01.h`](tft_setup_gc9a01.h) into `User_Setup.h`

### 4. Upload

1. Select board: **Tools → Board → ESP32 Dev Module**
2. Select port: **Tools → Port → (your ESP32 port)**
3. Click **Upload**

## Touch Configuration

The firmware now includes **auto-calibration** for the capacitive touch sensor. On boot, it reads the baseline touch value and sets a dynamic threshold, which fixes issues where touch stops working after re-flashing.

Debug touch values are printed to Serial (115200 baud) every 500ms — use the Serial Monitor to verify touch readings if you experience issues.

By default, the firmware uses ESP32's built-in capacitive touch. To use a digital touch sensor module (TTP223), change this line in `golubot-version8.ino`:

```cpp
#define USE_CAPACITIVE_TOUCH true
```

to:

```cpp
#define USE_CAPACITIVE_TOUCH false
```

## Mood System

Your Golubot has internal mood states that affect its behavior:

- **Energy** (green dot): Decreases over time, increases when fed
- **Happiness** (blue dot): Increases with interaction (taps), slowly decreases when ignored
- **Fullness**: Increases when fed, decreases when energy is low

When energy is low, Golubot falls asleep. When happiness is low, it becomes sad or bored. Keep interacting and feeding your pet to keep it happy!

The two small dots at the bottom of the face screen show current mood status:
- 🟢 Green = Good energy | 🟡 Yellow = Medium | 🔴 Red = Low
- 🔵 Cyan = Happy | 🟡 Yellow = Okay | 🟣 Magenta = Unhappy

## Upgrading from v8/v9

If you're upgrading from the previous version:

1. The display driver changed from generic TFT to **GC9A01** — update your `User_Setup.h`
2. Touch input now uses **capacitive sensing** instead of digital read
3. WiFi and WebServer libraries are no longer required
4. The face system is completely new with smooth animations
5. A mood system has been added that affects pet behavior

## License

Open source — feel free to modify and share!
