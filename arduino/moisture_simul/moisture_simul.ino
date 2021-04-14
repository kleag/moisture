/*
  Simple POST client for ArduinoHttpClient library
  Connects to server once every five seconds, sends a POST request
  and a request body

  created 14 Feb 2016
  modified 22 Jan 2019
  by Tom Igoe
  
  this example is in the public domain
 */
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
int moisture = 25;
const float DECAY = 0.82;
const int THRESHOLD = 12;
const int REFILL = 85;

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

void setup() {
  assert(THRESHOLD+REFILL <= 100);
  
  Serial.begin(9600);
  connectWifi();
}

void loop() {
  String kind;
  int value;
  if (moisture >= THRESHOLD) {
    moisture = constrain(int(moisture*DECAY) , 0, 100);
    kind = "moisture";
    value = moisture;
  }
  else {
    moisture += REFILL;    
    kind = "pump_time";
    value = 1;
  }
  Serial.print("New moisture is ");
  Serial.println(moisture);
  postEntry(kind, value);
  Serial.println("Wait 30 seconds");
  delay(30000);
}
