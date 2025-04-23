#include <WiFi.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <time.h>

// ----------- CONFIGURATION -----------
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN  23
#define CLK_PIN   18
#define CS_PIN     5

// -------- WIFI CREDENTIALS --------
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// -------- TIMEZONE & NTP --------
const char* ntpServer = "pool.ntp.org";
const char* timeZone_Miami = "EST5EDT,M3.2.0/2,M11.1.0/2";  // Miami Timezone (with DST)
const int   daylightOffset_sec = 0;   // Adjust if needed
const bool use12HourFormat = true;    // Toggle between 12H or 24H format

// Create Parola Display Object
MD_Parola matrixDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ----------- GLOBALS -----------
char timeString[6];  // Format: HH:MM
bool colonVisible = true;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;

void setup() {
  Serial.begin(115200);

  // Initialize the LED matrix display
  matrixDisplay.begin();
  matrixDisplay.setIntensity(5); // Brightness: 0–15
  matrixDisplay.setTextAlignment(PA_CENTER);
  matrixDisplay.displayClear();

  // Connect to WiFi access point
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  // Configure NTP time
  configTzTime(timeZone_Miami, ntpServer);
  Serial.print("Syncing NTP time");
  while (!time(nullptr)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" time synced!");
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      int hour = timeinfo.tm_hour;
      if (use12HourFormat) {
        hour = hour % 12;
        if (hour == 0) hour = 12;
      }

      // Toggle colon visibility every second
      colonVisible = !colonVisible;

      // Format time with or without colon
      sprintf(timeString, "%02d%c%02d", hour, 
              colonVisible ? ':' : ' ', timeinfo.tm_min);

      // Show the time via the serial.
      Serial.println(timeString);

      // Show the time in the display.
      matrixDisplay.displayText(timeString, PA_CENTER, 0, 0, 
                                PA_PRINT, PA_NO_EFFECT);

    } else {
      // Show error in the display.
      matrixDisplay.displayText("----", PA_CENTER, 0, 0, 
                                PA_PRINT, PA_NO_EFFECT);
    }

    // Animate all the zones in the display using the 
    // currently specified text and animation parameters.
    matrixDisplay.displayAnimate();
  }
}
