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
// Will send an alert if moisture does not raise after a pumping in the
// [ALERT_MIN, ALERT_MAX] interval in minutes
const int ALERT_MIN = 1;
const int ALERT_MAX = 20;
// Duratio of a pumping event in seconds
const int PUMPING_DURATION = 1;
// Delay in minutes between two measures and other actions
const int LOOP_DELAY = 10;
//const int LOOP_DELAY = 1;
// Will wait at least PUMP_DELAY minutes between two pumpings
const int PUMP_DELAY = 60;
//const int PUMP_DELAY = 1;


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
const String contentType = "application/x-www-form-urlencoded";

/** Connect to the WiFi network */
void connectWifi() {
  int status = WL_IDLE_STATUS;
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
  Serial.println("");
}

/** Post a message to the /entries/ API endpoint of the Web service
 * @param kind the type of the message. Should be one in [moisture, pump_time,
 * alert]
 * @param value int. Percent for moisture kind, duration in seconds for
 * pump_time and 0 for alert.
 * @return int. The status returned by the Web service
 */
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

int postAlert() {
  String postData;

  Serial.print("making POST alert request: ");
  client.beginRequest();
  int post_result = client.post("/entries/alert/");
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
  return statusCode;
}

/** handle diagnostic informations given by assertion and abort program
 * execution:
 */
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    abort();
}

/// Last pumping time in milliseconds
unsigned long lastPumping = millis();
/// Last measured moisture percent
int lastPercent = 0;

/// The LCD display
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/**
 * Pump water during @ref duration, send the event to the Web service and
 * @return the time of this pumping event
 */
unsigned long pump(int duration) {
    assert(duration < 100);
    char buffer[30];
    sprintf (buffer, "Pumping %d second(s)", duration);
    Serial.println(buffer);

    digitalWrite(RELAY_PIN, HIGH);
    delay(duration*1000);
    digitalWrite(RELAY_PIN, LOW);
    postEntry("pump_time", duration);
    return millis();
}

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
  sprintf (buffer, "%3d %%", percent);
//  sprintf (buffer, "%4d", val);

  Serial.println(buffer);
  lcd.print(buffer);
  postEntry("moisture", percent);
  return percent;
}

/**
 * Pause during @ref duration minutes
 * @param duration the number of minutes to wait
 */
void minutes_delay(int duration) {
  delay(1000*60*duration);
}

/**
 * Convert the @ref nb number of milliseconds to minutes
 * @param nb unsigned long, the number of milliseconds
 * @return unsigned long the number of minutes
 */
unsigned long millis2mn(unsigned long nb) {
  return nb/1000/60;
}

/**
 * Write an alert message on the LCD screen and post an alert entry to the Web
 * service
 */
void alert() {
  lcd.setCursor(0, 8);
  char buffer[10];
  sprintf (buffer, "ALERT");
  Serial.println(buffer);
  lcd.print(buffer);
  postEntry("alert", 0);
  postAlert();
}

/**
 * Set up the pump relay, prepare the LCD screen and connect to WiFi
 */
void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite(CONTRAST_PIN, CONTRAST);
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Humidite :");
  Serial.println("");
  connectWifi();

/*
  // Initial moisture level measure and send result
  int percent = measure();

  // if, moisture level is too low at start, pump some water
  if (percent < THRESHOLD)
  {
    lastPumping = pump(PUMPING_DURATION);
  }
  lastPercent = percent;

  // Wait 2 minutes to let the eventual first pumping to flow through the soil
*/
  delay(10000);
}

void loop() {
  // measure moisture level and send result
  int percent = measure();

  unsigned long currentTime = millis();
  unsigned long elapsed = millis2mn(currentTime - lastPumping);
  char buffer[100];
  sprintf (buffer, "In loop. Elapsed: %d mn. Percent: %3d %%. Last percent: %3d %%.", 
            elapsed, percent, lastPercent);
  Serial.println(buffer);

  // if, moisture level is too low, pump some water
  if (percent <= THRESHOLD && elapsed >= PUMP_DELAY)
  {
    lastPumping = pump(PUMPING_DURATION);
  }
  // check if moisture level has raised since the last pumping (if it occured
  // at least ALE RT_MIN mn ago (to let the water flow) and no more than
  // ALERT_MAX mn ago (such that it has not enough time to dry at all). If it
  // has not raised, write a message on LCD and send an alert message to the
  // Web service.
  else if (elapsed >= ALERT_MIN && elapsed <= ALERT_MAX && percent <= lastPercent) {
    alert();
  }
  lastPercent = percent;

  minutes_delay(LOOP_DELAY);
}
