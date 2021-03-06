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
const String serverAddress = "moisture.nsupdate.info";  // server address
const int port = 443;
//const String serverAddress = "192.168.1.21";  // server address
//const int port = 8000;

////////////////////////////////
// Hardware parmeters

// Pins used by your LCD screen
const int RS = 1, EN = 2, D4 = 3, D5 = 4, D6 = 5, D7 = 6;
// Other arduino pins used
const int RELAY_PIN = 7;
const int MOISTURE_PIN = A0;
// LCD screen contrast
const int CONTRAST_PIN = 0;
const int CONTRAST = 20;
const int MANUAL_PUMP_BUTTON_PIN = 8;

////////////////////////////////
// Software parameters

// MAX_DRY and MAX_WET: moisture sensor calibration. Check the measures given
// by your sensor whith completely wet and fully dry soils
const int MAX_WET = 506;
const int MAX_DRY = 867;
// Moisture percent under which pumping will be done
const int THRESHOLD = 15;
// Will send an alert if moisture does not raise after a pumping in the
// [ALERT_MIN, ALERT_MAX] interval in minutes
const int ALERT_MIN = 10;
const int ALERT_MAX = 30;
// Duratio of a pumping event in seconds
const int PUMPING_DURATION = 5;
// Delay in minutes between two measures and other actions in normal operation
const int LOOP_DELAY = 20*60*1000;
// Will wait at least PUMP_DELAY minutes between two pumpings
const int PUMP_DELAY = 60;


////////////////////////////////
// Secret parmeters

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
const String ssid = SECRET_SSID;    // your network SSID (name)
const String pass = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const String login = SECRET_LOGIN;        // your Web service user login
const String password = SECRET_PASSWORD;  // your Web service user password

WiFiSSLClient wifi;
//WiFiClient wifi;
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

/** Post a message to an API endpoint of the Web service
 * @param url String. The path part of the url.
 * @param data String. The data to post.
 * @return int. The status returned by the Web service
 */
int post(const String& urlPath, const String& data) {
  Serial.print("making POST request: ");
  Serial.println(data);
  client.beginRequest();
  int post_result = client.post(urlPath);
  Serial.print("Post url set: ");
  Serial.println(post_result);
  client.sendHeader("Content-Type", contentType);
  client.sendHeader("Content-Length", data.length());
  Serial.println("Headers ready");
  client.sendBasicAuth(login, password); // send the username and password for authentication
  Serial.println("Basic auth sent");
  client.beginBody();
  client.print(data);
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

/** Post a message to the /entries/ API endpoint of the Web service
 * @param raw int. The raw moisture level given by the sensor.
 * @param value int. Percent of moisture.
 * @return int. The status returned by the Web service
 */
int postEntry(int raw, int value) {
  char buffer[30];
  sprintf (buffer, "raw=%d&value=%d", raw, value);

  String postData = buffer;
  String urlPath = "/entries/";

  return post(urlPath, postData);
}

/** Post a message to the /entries/ API endpoint of the Web service
 * @param raw int. The raw moisture level given by the sensor.
 * @param value int. Percent of moisture.
 * @return int. The status returned by the Web service
 */
int postPumpingEntry(int duration) {
  char buffer[30];
  sprintf (buffer, "value=%d", duration);

  String postData = buffer;
  String urlPath = "/pumping_entries/";

  return post(urlPath, postData);

}

int postAlert() {
  String postData;
  String urlPath = "/alert/";

  return post(urlPath, postData);
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

unsigned int loopDelay = LOOP_DELAY;

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
    delay(500);
    postPumpingEntry(duration);
    minutes_delay(1);
    measure();
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
  sprintf (buffer, "%4d, %3d %%", val, percent);
//  sprintf (buffer, "%4d", val);

  Serial.println(buffer);
  lcd.print(buffer);
  postEntry(val, percent);
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
  lcd.setCursor(0, 12);
  char buffer[10];
  sprintf (buffer, "ALERT");
  Serial.println(buffer);
  lcd.print(buffer);
  postAlert();
}

#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
#define debounce                              10 // time to wait in milli secs

volatile  bool interrupt_process_status = {
  !triggered                                     // start with no switch press pending, ie false (!triggered)
};
bool initialisation_complete =            false; // inhibit any interrupts until initialisation is complete

//
// ISR for handling interrupt triggers arising from associated button switch
//
void button_interrupt_handler()
{
  if (initialisation_complete == true)
  { //  all variables are initialised so we are okay to continue to process this interrupt
    if (interrupt_process_status == !triggered) {
      // new interrupt so okay start a new button read process -
      // now need to wait for button release plus debounce period to elapse
      // this will be done in the button_read function
      if (digitalRead(MANUAL_PUMP_BUTTON_PIN) == HIGH) {
        // button pressed, so we can start the read on/off + debounce cycle wich will
        // be completed by the button_read() function.
        interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
        loopDelay = 500; // we will loop quickly until switching and action are done
      }
    }
  }
} // end of button_interrupt_handler

bool read_button() {
  int button_reading;
  // static variables because we need to retain old values between function calls
  static bool     switching_pending = false;
  static long int elapse_timer;
  if (interrupt_process_status == triggered) {
    // interrupt has been raised on this button so now need to complete
    // the button read process, ie wait until it has been released
    // and debounce time elapsed
    button_reading = digitalRead(MANUAL_PUMP_BUTTON_PIN);
    if (button_reading == HIGH) {
      // switch is pressed, so start/restart wait for button relealse, plus end of debounce process
      switching_pending = true;
      elapse_timer = millis(); // start elapse timing for debounce checking
    }
    if (switching_pending && button_reading == LOW) {
      // switch was pressed, now released, so check if debounce time elapsed
      if (millis() - elapse_timer >= debounce) {
        // dounce time elapsed, so switch press cycle complete
        switching_pending = false;             // reset for next button press interrupt cycle
        interrupt_process_status = !triggered; // reopen ISR for business now button on/off/debounce cycle complete
        return switched;                       // advise that switch has been pressed
      }
    }
  }
  return !switched; // either no press request or debounce period not elapsed
} // end of read_button function

void manualPump() {
  Serial.println("manualPump");
  pump(5);
}

/**
 * Set up the pump relay, prepare the LCD screen and connect to WiFi
 */
void setup() {
  pinMode(MANUAL_PUMP_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MANUAL_PUMP_BUTTON_PIN),
                  button_interrupt_handler,
                  interrupt_trigger_type);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite(CONTRAST_PIN, CONTRAST);
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Moisture:");
  Serial.println("");
  connectWifi();

  // Wait some time to let the eventual first pumping to flow through the soil
  delay(2000);
  initialisation_complete = true; // open interrupt processing for business
}

void loop() {
  // test buton switch and process if pressed
  if (read_button() == switched) {
    Serial.println("Manual pump button has been pressed");
    // button on/off cycle now complete,
    manualPump();
    loopDelay = LOOP_DELAY; // switch back to default delay 
  }
  else {
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
    // ALERT_MAX??mn ago (such that it has not enough time to dry at all). If it
    // has not raised, write a message on LCD and send an alert message to the
    // Web service.
    else if (elapsed >= ALERT_MIN && elapsed <= ALERT_MAX
        && percent <= THRESHOLD && percent <= lastPercent) {
      alert();
    }
    lastPercent = percent;
  }
  delay(loopDelay);
}
