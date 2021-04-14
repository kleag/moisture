#define __ASSERT_USE_STDERR

#include <assert.h>

#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include <LiquidCrystal.h>

// Put your secret data in this file (see SECRET_* below)
#include "arduino_secrets.h"

////////////////////////////////
// Web service parmeters

// Set where you moistureduino Web service is listening
const String serverAddress = "192.168.1.21";  // server address
const int port = 8000;

////////////////////////////////
// Hardware parmeters

// Pins used by your LCD screen
const int RS = 0, EN = 1, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
// Other arduino pins used
const int CONTRAST_PIN = 6;
const int RELAY_PIN = 13;
const int MOISTURE_PIN = A1;
// LCD screen contrast
const int CONTRAST = 20;

////////////////////////////////
// Software parameters

// MAX_DRY and MAX_WET: moisture sensor calibration. Check the measures given
// by your sensor whith completely wet and fully dry soils
const int MAX_WET = 485;
const int MAX_DRY = 955;
// Moisture percent under which pumping will be done
const int THRESHOLD = 10;
// Delay in minutes between two measures and other actions
const int LOOP_DELAY = 10;
// Will wait at least PUMP_DELAY minutes between two pumpings
const int PUMP_DELAY = 60;
// Will send an alert if moisture does not raise after a pumping in the
// [ALERT_MIN, ALERT_MAX] interval in minutes
const int ALERT_MIN = 5;
const int ALERT_MAX = 20;
// Duratio of a pumping event in secondes
const int PUMPING_DURATION = 1;


////////////////////////////////
// Secret parmeters

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
const String ssid = SECRET_SSID;    // your network SSID (name)
const String pass = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const String login = SECRET_LOGIN;        // your Web service user login
const String password = SECRET_PASSWORD;  // your Web service user password


WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;
const String contentType = "application/x-www-form-urlencoded";

void connectWifi() {
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}


int postEntry(const String& kind, int value) {
  assert(kind.length() < 30);
  char buffer[30];
  sprintf (buffer, "kind=%s&value=%d", kind.c_str(), value);

  String postData = buffer;

  Serial.print("making POST request: ");
  Serial.println(postData);
  client.beginRequest();
  int post_result = client.post("/entries/");
  Serial.print("Post url set: ");
  Serial.println(post_result);
  client.sendHeader("Content-Type", contentType);
  client.sendHeader("Content-Length", postData.length());
  Serial.println("Headers ready");
  client.sendBasicAuth(login, password); // send the username and password for authentication
  Serial.println("Basic auth sent");
  client.beginBody();
  client.print(postData);
  Serial.println("Body written");
  client.endRequest();
  Serial.println("Request sent");


  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
//  String response = client.responseBody();
//  Serial.print("Response: ");
//  Serial.println(response);
  return statusCode;
}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    abort();
}

// Last pumping time in milliseconds
unsigned long lastPumping = millis();
int lastPercent = 0;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  Serial.begin(9600);           //  setup serial
  connectWifi();
  pinMode(CONTRAST_PIN, OUTPUT);  // sets the pin as output
  pinMode(RELAY_PIN, OUTPUT);  // sets the pin as output
  analogWrite(CONTRAST_PIN, CONTRAST);
  digitalWrite(RELAY_PIN, LOW);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Humidite :");
}

/**
 * Pump water during @ref duration, send the event to the Web service and
 * @return the time of this pumping event
 */
unsigned long pump(int duration) {
    digitalWrite(RELAY_PIN, LOW);
    delay(duration*1000);
    digitalWrite(RELAY_PIN, HIGH);
    postEntry("pump_time", duration);
    return millis();
}

int measure() {
  int val = analogRead(MOISTURE_PIN);  // read the input pin
  int percent = constrain(map(val, MAX_DRY, MAX_WET,0, 100) , 0, 100);
  lcd.setCursor(0, 1);
  char buffer[10];
  sprintf (buffer, "%3d %%", percent);
//  sprintf (buffer, "%4d", val);

  Serial.println(buffer);
  lcd.print(buffer);
  postEntry("moisture", percent);
  return percent;
}

void minutes_delay(int duration) {
  delay(1000*60*duration);
}

unsigned long millis2mn(unsigned long nb) {
  return nb/1000/60;
}

void alert() {
  lcd.setCursor(0, 8);
  char buffer[10];
  sprintf (buffer, "ALERT");
  Serial.println(buffer);
  lcd.print(buffer);
  postEntry("alert", 0);
}

void loop() {
  // measure moisture level and send result
  int percent = measure();

  unsigned long currentTime = millis();
  unsigned long elapsed = millis2mn(currentTime - lastPumping);

  // check if moisture level has raised since the last pumping (if it occured
  // at least ALERT_MIN mn ago (to let the water flow) and no more than
  // ALERT_MAX mn ago (such that it has not enough time to dry at all). If it
  // has not raised, write a message on LCD and send an alert message to the
  // Web service.
  if (elapsed > ALERT_MIN && elapsed <= ALERT_MAX && percent <= lastPercent) {
    alert();
  }
  // if, moisture level is too low, pump some water
  if (percent < THRESHOLD && elapsed > PUMP_DELAY)
  {
    lastPumping = pump(PUMPING_DURATION);
  }
  lastPercent = percent;

  minutes_delay(LOOP_DELAY);
}
