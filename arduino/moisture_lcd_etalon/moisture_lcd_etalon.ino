#include <LiquidCrystal.h>

////////////////////////////////
// Hardware parmeters

// Pins used by your LCD screen
const int RS = 1, EN = 2, D4 = 3, D5 = 4, D6 = 5, D7 = 6;
// Other arduino pins used
const int MOISTURE_PIN = A0;
// LCD screen contrast
const int CONTRAST_PIN = 0;
const int CONTRAST = 20;

////////////////////////////////
// Software parameters

// MAX_DRY and MAX_WET: moisture sensor calibration. Check the measures given
// by your sensor whith completely wet and fully dry soils
const int MAX_WET = 506;
const int MAX_DRY = 867;

/// The LCD display
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/**
 * Measure the moisture level, write the result on the LCD screen and post the
 * result to the Web service
 * @result int, the measured moisture percent
 */
int measure() {
  int val = analogRead(MOISTURE_PIN);  // read the input pin
  int percent = constrain(map(val, MAX_DRY, MAX_WET,0, 100) , 0, 100);
  lcd.setCursor(0, 1);
  char buffer[10];
  sprintf (buffer, "%4d, %3d %%", val, percent);

  Serial.println(buffer);
  lcd.print(buffer);
  return percent;
}

/**
 * Set up the pump relay, prepare the LCD screen and connect to WiFi
 */
void setup() {
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite(CONTRAST_PIN, CONTRAST);
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Moisture:");
  Serial.println("");
}

void loop() {
  // measure moisture level and send result
  int percent = measure();

  delay(5000);
}
