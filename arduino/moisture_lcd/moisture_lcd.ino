#define __ASSERT_USE_STDERR
  
#include <assert.h>

#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
const String ssid = SECRET_SSID;    // your network SSID (name)
const String pass = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const String login = SECRET_LOGIN;    // your network SSID (name)
const String password = SECRET_PASSWORD;    // your network password (use for WPA, or use as key for WEP)

const String serverAddress = "192.168.1.21";  // server address
const int port = 8000;

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


// include the library code:
#include <LiquidCrystal.h>

const int RELAY_PIN = 13;
const int analogPin = A1; 
const int MAX_WET = 485;
const int MAX_DRY = 955;
const int THRESHOLD = 10;

int val = 0;  
int percent = 0;
unsigned long lastPumping = millis();

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;

const int CONTRAST = 20;
const int CONTRAST_PIN = 6;

char buffer[10];
bool first = 1;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);           //  setup serial
  connectWifi();
  pinMode(CONTRAST_PIN, OUTPUT);  // sets the pin as output
  pinMode(RELAY_PIN, OUTPUT);  // sets the pin as output
  analogWrite(CONTRAST_PIN, CONTRAST);
  digitalWrite(RELAY_PIN, HIGH);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Humidite :");
}

void pump(int duration) {
    digitalWrite(RELAY_PIN, LOW);
    delay(duration*1000);
    digitalWrite(RELAY_PIN, HIGH);
    postEntry("pump_time", duration);
    lastPumping = millis();
}

void loop() {
  val = analogRead(analogPin);  // read the input pin
  percent = constrain(map(val, MAX_DRY, MAX_WET,0, 100) , 0, 100);
  lcd.setCursor(0, 1);
  sprintf (buffer, "%3d %%", percent);
//  sprintf (buffer, "%4d", val);

  Serial.println(buffer);
  lcd.print(buffer);
  postEntry("moisture", percent);
  delay(1000*60*10);

  unsigned long currentTime = millis();
  unsigned long elapsed = (currentTime - lastPumping)/1000/60;
  if (percent < THRESHOLD && elapsed > 60)
  {
    pump(1);
  }
}
