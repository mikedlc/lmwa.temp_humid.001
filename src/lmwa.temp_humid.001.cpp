//This code for D1 Mini + SSD1306 128x64 OLED + DHT22 --> tago.io

//1.3 OLED SSH1106 Includes & setup
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

void httpRequest();
void printWifiStatus();

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Set up DHT22 Temp Sensor
#include "DHT.h"
#define DHTPIN 0
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

//Wifi Stuff
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char *ssid =	"LMWA-PumpHouse";		// cannot be longer than 32 characters!
const char *pass =	"ds42396xcr5";		//
//const char *ssid =	"WiFiFoFum";		// cannot be longer than 32 characters!
//const char *pass =	"6316EarlyGlow";		//

WiFiClient client;

//Tago.io server address:
char server[] = "api.tago.io";
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 300L * 1000L; // delay between updates, in milliseconds
String Device_Token = "b7507450-0db3-4a6d-8fe8-6cea8192853f";
String temperature_string = "";
String humidity_string = "";

int counter = 1;
char farenheitout[32];
String farenheittoprint;
char humidityout[32];
String humiditytoprint;
String wifistatustoprint;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;                     // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println();
  Serial.println();

  //Init DHT22
  dht.begin();

  //1.3" OLED Setup
  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
 
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // draw a single pixel
  display.drawPixel(10, 10, SH110X_WHITE);
  // Show the display buffer on the hardware.
  // NOTE: You _must_ call display after making any drawing commands
  // to make them visible on the display hardware!
  display.display();
  delay(2000);
  display.clearDisplay();

}


/*
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Failure is always an option");
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();
  delay(2000);
  display.clearDisplay();
  */

void loop() {

  //Wifi Stuff
  if (WiFi.status() != WL_CONNECTED) {

    //Write wifi connection to display
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Cabinet Temp Sensor");
    display.setTextSize(1);
    display.println(" ");
    display.println("Connecting To WiFi:");
    display.println(ssid);
    display.println(" ");
    display.println("Wait for it......");
    display.display();

    //write wifi connection to serial
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.setHostname("LMWAITCABINET");
    WiFi.begin(ssid, pass);

    //delay 8 seconds for effect
    delay(8000);

    if (WiFi.waitForConnectResult() != WL_CONNECTED){
      return;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Cabinet Temp Sensor");
    display.setTextSize(1);
    display.println(" ");
    display.println("Connected To WiFi:");
    display.println(ssid);
    display.println(" ");
    display.display();

    Serial.println("\n\nWiFi Connected! ");
    printWifiStatus();

  }

  if (WiFi.status() == WL_CONNECTED) {
    wifistatustoprint="WiFi Connected!";
  }else{
    wifistatustoprint="Womp, No WiFi!";
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Cabinet Temp Sensor");
    display.setTextSize(1);
    display.println(" ");
    display.println("Connected to WiFi:");
    display.println(ssid);
    display.println(" ");
    display.println("DHT Sensor Error!");
    display.display();

    //delay 2 seconds for effect
    delay(2000);
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //Convert floats to strings
  dtostrf(f, 8, 2, farenheitout);
  dtostrf(h, 8, 2, humidityout);
  farenheittoprint = "Temperature:" + String(farenheitout) + "F";
  humiditytoprint = "Humidity:   " + String(humidityout) + "%";

  // clear the display
  display.clearDisplay();

  //prep buffer to update display

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Cabinet Temp Sensor");
  display.setTextSize(1);
  display.println(" ");
  display.println(farenheittoprint);
  display.println(humiditytoprint);
  display.println(" ");
  display.println(wifistatustoprint);
  display.print("SSID:");
  display.println(ssid);
  display.print("IP:");
  display.println(WiFi.localIP());
  // write the buffer to the display
  display.display();


  // if display interval has passed since your last connection,
  // then connect again and send data to tago.io
  if (millis() - lastConnectionTime > postingInterval) {

    Serial.println("Time to post to tago.io!");

    int temperature_int = (int) f;  //convert data format from float to int
    temperature_string =String(temperature_int); //then to string
    
    int humidity_int = (int) h;
    humidity_string = String(humidity_int);

    // then, send data to Tago
    httpRequest();
  }

  counter++;

  //wait 5 before next cycle
  delay(5000);
}

// this method makes a HTTP connection to tago.io
void httpRequest() {

  Serial.println("Sending this Temperature:");
  Serial.println(temperature_string);
  Serial.println("Sening this Humidity:");
  Serial.println(humidity_string);
  
    // close any connection before send a new request.
    // This will free the socket on the WiFi shield
    client.stop();

    Serial.println("Starting connection to server for Temperature...");
    // if you get a connection, report back via serial:
    String PostTemperature = String("{\"variable\":\"itcabinet_temperature\", \"value\":") + String(temperature_string)+ String(",\"unit\":\"F\"}");
    String Dev_token = String("Device-Token: ")+ String(Device_Token);
    if (client.connect(server,80)) {                      // we will use non-secured connnection (HTTP) for tests
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.println("POST /data? HTTP/1.1");
    client.println("Host: api.tago.io");
    client.println("_ssl: false");                        // for non-secured connection, use this option "_ssl: false"
    client.println(Dev_token);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(PostTemperature.length());
    client.println();
    client.println(PostTemperature);
    Serial.println("Temperature sent!\n");
    }  else {
      // if you couldn't make a connection:
      Serial.println("Server connection failed.");
    }

    client.stop();

    Serial.println("Starting connection to server for Humidity...");
    // if you get a connection, report back via serial:
    String PostHumidity = String("{\"variable\":\"itcabinet_humidity\", \"value\":") + String(humidity_string)+ String(",\"unit\":\"%\"}");
    if (client.connect(server,80)) {                      // we will use non-secured connnection (HTTP) for tests
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.println("POST /data? HTTP/1.1");
    client.println("Host: api.tago.io");
    client.println("_ssl: false");                        // for non-secured connection, use this option "_ssl: false"
    client.println(Dev_token);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(PostHumidity.length());
    client.println();
    client.println(PostHumidity);
    Serial.println("Humidity sent!\n");
  }
  else {
    // if you couldn't make a connection:
    Serial.println("Server connection failed.");
  }

    // note the time that the connection was made:
    lastConnectionTime = millis();
}

//this method prints wifi network details
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("Hostname: ");
  Serial.println(WiFi.getHostname());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println("");
}

