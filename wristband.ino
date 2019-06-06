//Libraries
#include <FastLED.h>          // for leds
#include <SoftwareSerial.h>   // for bluetooth

//Pins
#define LED_PIN     3             // LEDs connected digital pin 3
#define VIBRO_MOTOR_PIN   6       // vibration motor connected digital pin 6
int mic_Pin = A7;                 // microphone connected analog pin 7
const int brightness_button = 2;  // push-button to control brightness connected digital pin 2
const int mode_button = 4;        // push-button to control modes connected digital pin 4

//Leds configurations
#define NUM_LEDS    8
#define BRIGHTNESS  100
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

//Bluetooth configurations
const long baudRate = 9600;      // baud rate for bluetooth communication
char c = ' ';                    // value that bluetooth send
SoftwareSerial BTserial(10, 11); // pins that are used instead of RX and TX pins

//Start point for buttons
int mode = 1; // 1 - leds & vibr / 2 - only vibr / 3 - only leds /
int brightness_b = 10;

//DB meter
int i;
int sensorValue = 0; // value to store the value coming from the sensor
const int sampleWindow = 50; // sample window width in mS (50mS = 20Hz)
unsigned int sample;  // value that we get in analog input where microphone is connected
float db_value;  // used to save db value and send it through bluetooth also used in notification function
int db_min = 88; // max value of the "good" sound level
int db_max = 93; // actually max value should be 100, but since with this microphone we could only get max value 95, so we choose from 93db that it starts to be very loud.

void setup() {
  //setup for leds
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  //setup for vibration motor and two push-buttons
  pinMode(VIBRO_MOTOR_PIN, OUTPUT);
  pinMode(brightness_button, INPUT_PULLUP);
  pinMode(mode_button, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");

  BTserial.begin(baudRate);
  Serial.print("BTserial started at "); Serial.println(baudRate);
  Serial.println(" ");
}

void loop() {
  mode = change_mode();         // checking/changing the mode
  change_brightness();          // checking/changing brightness level
  db_value = db_meter();        // getting db value
  BTserial.write(db_value);     // send this db value to the case
  notification(db_value, mode); // giving notification depending on the db value and mode that we got before
  delay(1000);                  // so we wait 1s so it wouldn't be a spam for second device
}

//---------------------- db meter -------------------------------
float db_meter() {
  unsigned long startMillis = millis();     // Start of sample window
  float peakToPeak = 0;                     // peak-to-peak level
  unsigned int signalMax = 0;               // minimum value
  unsigned int signalMin = 1024;            // maximum value

  while (millis() - startMillis < sampleWindow) { // collect data for 50mS
    sample = analogRead(mic_Pin);
    if (sample < 1024) {
      if (sample > signalMax) {
        signalMax = sample;
      }
      else if (sample < signalMin) {
        signalMin = sample;
      }
    }
  }
  peakToPeak = signalMax - signalMin;
  float db = map(peakToPeak, 20, 900, 49.5, 90);
  Serial.print(db);
  Serial.println(" db");
  return db;
}

//--------------------- notifications ---------------------------
void notification(float db, int mode2) {

  //if db level is lower db_min
  if (db < db_min) {
    if (mode2 == 1) { // mode 1: leds and vibr: but there will be no vibration because sound level is "good" under 88 db
      leds_color(0, 100, 0);
    }
    else if (mode2 == 2) { // mode 2: only vibr: since sound level is "good", we need only to turn off leds
      leds_color(0, 0, 0);
    }
    else if (mode2 == 3) { // mode 3: only leds
      leds_color(0, 100, 0);
    }
  }

  //if db level is between min and max value
  else if (db > db_min && db < db_max) {
    if (mode2 == 1) { // mode 1: leds and vibr: but there will be no vibration because sound level is "medium" (88-93)
      leds_color(100, 100, 0);
    }
    else if (mode2 == 2) { // mode 2: only vibr: since sound level is "medium", we need only to turn off leds
      leds_color(0, 0, 0);
    }
    else if (mode2 == 3) { // mode 3: only leds
      leds_color(100, 100, 0);
    }
  }

  //if db level higher than max value
  else if (db > db_max) {
    if (mode2 == 1) { // mode 1: leds & vibr
      leds_color(100, 0, 0);
      vibr();
    }
    else if (mode2 == 2) { // mode 2: only vibr, but again we need to turn off leds
      vibr();
      leds_color(0, 0, 0);
    }
    else if (mode2 == 3) { // mode 3: only leds
      leds_color(100, 0, 0);
    }
  }
}

//----------------------- Help functions ------------------------
// to skip writing the same code in the some part we have two helping functions.

// simple function for vibration
void vibr() {
  digitalWrite(VIBRO_MOTOR_PIN, HIGH);
  delay(500);
  digitalWrite(VIBRO_MOTOR_PIN, LOW);
  delay(500);
}

// simple function for leds, that light all leds depending on the input values
void leds_color(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS ; ++i) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

//--------------- Brightness and mode change --------------------
void change_brightness() {
  if (digitalRead(brightness_button) == LOW) {
    if (brightness_b <= 100) { // if button is pressed and brightness less 100% we make it 10% lighter.
      brightness_b = brightness_b + 10;
      FastLED.setBrightness(brightness_b);
    }
    else { // if button is pressed and brightness 100%, it will be sent back to 10%
      brightness_b = 10;
      FastLED.setBrightness(brightness_b);
    }
  }
}

// if button pressed change mode and return mode number
int change_mode() {
  if (digitalRead(mode_button) == LOW) {
    if (mode <= 3) {
      mode++;
    }
    else {
      mode = 1;
    }
  }
  return mode;
}
