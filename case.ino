//for bluetooth
#include <SoftwareSerial.h> // library
SoftwareSerial BTserial(10, 11); // pins that are used instead of RX and TX pins
const long baudRate = 9600; // baud rate for bluetooth communication
char c = ' '; // value that bluetooth receive

//for screen
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
// screen connected pins
#define TFT_CS     3
#define TFT_RST    2
#define TFT_DC     4
#define TFT_SCLK 6
#define TFT_MOSI 5
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int db_min = 88; // max value when sound level is good
int db_max = 93; // actually max value should be 100, but since with this microphone we could only get max value 95, so we choose from 93db that it starts to be very loud.

int db = 0; // db value on the screen
int db_min_s = 0; // count second for "Good" level
int db_med_s = 0; // count second for "Medium" level
int db_max_s = 0; // count second for "Loud" level

// values that are used for converting second to min/hours/days
static uint16_t days;
static uint8_t hours, minutes, seconds;

//---------------------- setup -------------------------------
void setup() {
  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");

  BTserial.begin(baudRate);
  Serial.print("BTserial started at "); Serial.println(baudRate);
  Serial.println(" ");

  Serial.println("Hello! ST7735 TFT Test");
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  Serial.println("Initialized");
  tft.fillScreen(ST7735_BLACK);
}

//---------------------- loop -------------------------------
void loop() {
  tftPrint(); // prints information to the screen
  if (BTserial.available()) { // receiving db value through the bluetooth
    c = BTserial.read();
    db = (int) c; // converting char to int
  }
  count(db); // counting
  delay(500);
}

//---------------------- count secound -------------------------------
// counting seconds depending on db level
void count(int db2) {
  if (db <= db_min) {
    db_min_s++;
  }
  else if (db > db_min && db < db_max) {
    db_med_s++;
  }
  else if (db >= db_max) {
    db_max_s++;
  }
}

//---------------------- second to min/hour converter -------------------------------
static void TimeCalc(uint32_t  secs) {
  seconds = secs % (uint32_t)60;
  secs /= (uint32_t)60;
  minutes = secs % (uint32_t)60;
  secs /= (uint32_t)60;
  hours = secs % (uint32_t)24;
}

//---------------------- print information to the screen -------------------------------
void tftPrint() {
  tft.fillRoundRect(34, 0, 100, 20, 0, ST7735_BLACK);  // clean previous good value
  tft.fillRoundRect(34, 30, 100, 20, 0, ST7735_BLACK); // clean previous mid value
  tft.fillRoundRect(34, 60, 100, 20, 0, ST7735_BLACK); // clean previous loud value
  tft.fillRoundRect(34, 90, 100, 20, 0, ST7735_BLACK); // clean previous db value

  tft.setCursor(0, 0);
  tft.setTextSize(2);

  // "Good sound" info
  TimeCalc(db_min_s);
  tft.setTextColor(ST7735_GREEN);
  tft.print("G: ");
  if (db_min_s < 60) {
    tft.print(seconds);
    tft.println(" s");
    tft.println(" ");
  }
  else if (db_min_s >= 60 && db_min_s < 3600) {
    tft.print(minutes);
    tft.println(" min");
    tft.println(" ");
  }
  else if (db_min_s > 3600) {
    tft.print(hours);
    tft.println(" h");
    tft.println(" ");
  }

  // "Medium sound" info
  TimeCalc(db_med_s);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("M: ");
  if (db_min_s < 60) {
    tft.print(seconds);
    tft.println(" s");
    tft.println(" ");
  }
  else if (db_med_s >= 60 && db_med_s < 3600) {
    tft.print(minutes);
    tft.println(" min");
    tft.println(" ");
  }
  else if (db_med_s > 3600) {
    tft.print(hours);
    tft.println(" h");
    tft.println(" ");
  }

  // "Loud sound" info
  TimeCalc(db_max_s);
  tft.setTextColor(ST7735_RED);
  tft.print("L: ");
  if (db_max_s < 60) {
    tft.print(seconds);
    tft.println(" s");
    tft.println(" ");
  }
  else if (db_max_s >= 60 && db_max_s < 3600) {
    tft.print(minutes);
    tft.println(" min");
    tft.println(" ");
  }
  else if (db_max_s > 3600) {
    tft.print(hours);
    tft.println(" h");
    tft.println(" ");
  }

  // Db value
  TimeCalc(db);
  tft.setTextColor(ST7735_BLUE);
  tft.print("DB: ");
  tft.println(db);
}
