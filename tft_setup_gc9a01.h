// ==========================================
// TFT_eSPI Configuration for GC9A01
// ==========================================
//
// Copy these settings into your TFT_eSPI
// User_Setup.h file located at:
//   Arduino/libraries/TFT_eSPI/User_Setup.h
//
// Make sure to comment out any existing
// driver definitions before pasting.
// ==========================================

#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// --- ESP32 SPI Pin Assignments ---
#define TFT_MISO -1     // Not used for GC9A01
#define TFT_MOSI 23     // SDA pin
#define TFT_SCLK 18     // SCL pin
#define TFT_CS    5     // Chip Select
#define TFT_DC   16     // Data/Command
#define TFT_RST  17     // Reset
#define TFT_BL    4     // Backlight (optional, set -1 if unused)

// --- Font Loading ---
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// --- SPI Frequency ---
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
