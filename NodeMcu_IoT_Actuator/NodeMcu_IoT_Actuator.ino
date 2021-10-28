// Used board setting:
// WeMos D1 R1

//#define SHOW_OUTPUTS_FOR_DISLPAY_IN_SERIAL_TOO

#define USE_SECRET  1
#define USE_MOCK

// 1 "TP-Link_BB"    
// 2 "BB_Home2"  
// 3 "P20 Pro"    
// 4 "BB_guest"

//#define USE_TEST_CHANNEL
#define SKIP_TS_COMMUNICATION

#define VERSION                   "v0.2"
#define BUILDNUM                       8

#define SERIAL_BOUND_RATE         115200
#define SOFT_SERIAL_BOUND_RATE      9600

// Pinout: https://circuits4you.com/2017/12/31/nodemcu-pinout/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

#include "secrets.h"
#include "params.h"

#include "actuator_ui2.h"


//==================================
// WIFI
//==================================
const char* ssid =           S_SSID;
const char* password =       S_PASS;



// =================================
// HTTP
// =================================
ESP8266WebServer server(SERVER_PORT);
HTTPClient http;


//==================================
// ThingSpeak
//==================================
WiFiClient client;

//==================================
// EEPROM
//==================================
int eepromAddr =                   1;
int eepromAddr2 =                  4;


//==================================
// "Data" variables
//==================================

short action = NOTHING; // can be NOTHING or HEATING

//==================================
// Other variables
//==================================
long lastWiFiCheck = 0;
long lastCheck = 0;
int errorCount = 0;

unsigned long elapsedTime = 0;


// ==========================================================================================================================
//                                                 Hw feedback function
// ==========================================================================================================================



// ==========================================================================================================================
//                                                     Other methods
// ==========================================================================================================================

void doRestart(){
  Serial.println("Restart now...");
  ESP.restart();
}


void saveToEEPROM(int addr, short value){

  byte b1 = value & 0b11111111;
  byte b2 = (value>>8) & 0b11111111;

  Serial.println("Value: " + String(value));
  Serial.println("b1: " + String(b1));
  Serial.println("b2: " + String(b2));
  
  EEPROM.write(addr, b1);
  EEPROM.write(addr+1, b2);
  EEPROM.commit();
}

short loadFromEEPROM(int addr){
  byte b1 = EEPROM.read(addr);
  byte b2 = EEPROM.read(addr+1);

  short value = 0;
  value |= (b2<<8);
  value |= b1;
  return value;
}

void turnHeating(short state){

   if(state == ON){
    digitalWrite(RELAY_PIN, HIGH);   // due to using BC337 NPN transistor on output
  }else{
    digitalWrite(RELAY_PIN, LOW);  
  }
  
  //TODO HERE
}

// ==========================================================================================================================
//                                                  Web server functions
// ==========================================================================================================================

void HandleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", message);
}

/*
void turnOutput(){
  String value = server.arg("value"); //this lets you access a query param (http://x.x.x.x/set?value=12)
  int v = value.toInt();
  if(v == 1){
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Out: high");
  }else{
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Out: low");
  }
  String m = String("Value received:\n") + String(v);
  server.send(200, "text/html", m );
}
/**/


void HandleNotRstEndpoint(){
  doRestart();
}

void HandleRoot(){
  String message = "";
  message = getHTML(action);
  /*
  message = generateHtmlHeader();
  message += generateHtmlBody();
  /**/
  server.send(200, "text/html", message );
}

String getNeededAction(){
    // old
    //http.begin("192.168.1.170:81/data");  //Specify request destination

    WiFiClient wc;
    
    String thermostatServerPath = "http://" + String(THERMOSTAT_IP) + "/result";
    //String thermostatServerPath = "http://192.168.1.141:8083/result";
    //String thermostatServerPath = "http://192.168.1.107/result";
    http.begin(wc, thermostatServerPath.c_str());  //Specify request destination

    int httpCode = http.GET();                                  //Send the request
    
    String payload = "";
    Serial.println("httpCode: " + String(httpCode));
    if (httpCode != 0) { //Check the returning code
      payload = http.getString();   //Get the request response payload
    }else{
      payload = String(NEEDED_ACTION_UNKNOWN);
      //Serial.println("httpCode: " + String(httpCode));
    }
    //Serial.println("");
 
    http.end();   //Close connection
    Serial.println("Wanted action: " + payload);

    return payload;
}

void HandleData(){
  //Serial.println("HandleData called...");
  String message = "TODO here";
  server.send(200, "text/html", message );
}



// ==========================================================================================================================
//                                                        WiFi handling
// ==========================================================================================================================

int connectToWiFi(String ssid, String pw){
  // Setup WIFI
  WiFi.begin(ssid, password);
  Serial.println("Try to connect (SSID: " + String(ssid) + ")");
  
  // Wait for WIFI connection
  int tryCount = 0;
  while ( (WiFi.status() != WL_CONNECTED) && (tryCount < WIFI_TRY_COUNT)) {
    delay(WIFI_WAIT_IN_MS);
    tryCount++;
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED){
    return 1;
  }else{
    return 0;
  }
}

int initWiFi(){
  
  if(connectToWiFi(ssid, password)){
    Serial.println("");
    Serial.println("Connected");
    String ipMessage = "I" + WiFi.localIP().toString();
    //Serial.println("SENT: " + ipMessage );
    
    // for reconnecting feature
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.hostname(String(SOFTWARE_NAME) + " " + String(MODULE_NAME) + " " + String(VERSION) + " b" + String(BUILDNUM));
  }else{
    Serial.println("");
    Serial.print("ERROR: Unable to connect to ");
    Serial.println(ssid);
    doRestart();
  }
}


// ==========================================================================================================================
//                                                        Init
// ==========================================================================================================================
 
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  
  turnHeating(OFF);

  elapsedTime = millis();
  errorCount = 0;
  
  Serial.begin(SERIAL_BOUND_RATE);
  Serial.println();

  // EEPROM
  EEPROM.begin(128);
  // EEPROM read methods can not be used before the first write method..
  //tempSet = loadFromEEPROM(eepromAddr);

  initWiFi();

  Serial.println("\n");
  Serial.println(String(SOFTWARE_NAME) + " " + String(VERSION));
  #ifdef USE_MOCK
    Serial.println("MOCK USED!");
  #endif
  Serial.println("Thermostat IP: " + String(THERMOSTAT_IP));


  server.on("/", HandleRoot);
  server.on("/data", HandleData);
  //server.on("/set", turnOutput);
  //server.on ("/save", handleSave);
  server.on("/rst", HandleNotRstEndpoint);
  server.onNotFound( HandleNotFound );
  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString() + String("@ port: ") + String(SERVER_PORT) );


  //TODO
  delay(500);
}


// ==========================================================================================================================
//                                                        Loop
// ==========================================================================================================================

void wifiConnectionCheck(long now){
  //TODO: need to add optional macro for Serial prints of WiFi state
  if( (now - lastWiFiCheck) >= DELAY_BETWEEN_WIFI_CONNECTION_STATUS_CHECKS_IN_MS ){
    lastWiFiCheck = now;
    
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL:
        Serial.println("Configured SSID cannot be reached");
        break;
      case WL_CONNECTED:
        //Serial.println("Connection successfully established");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("Connection failed");
        //initWiFi();
        break;
    }
    /*
    Serial.printf("Connection status: %d\n", WiFi.status());
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
    /**/
  }
}


void doOurTask(long now){
  if( (now - lastCheck) > DELAY_BETWEEN_ITERATIONS_IN_MS ){ //Take a measurement at a fixed time (durationTemp = 5000ms, 5s)
    
    lastCheck = now;
    elapsedTime = now;
  
    String res = getNeededAction();

    if(res == NEEDED_ACTION_HEATING){
      turnHeating(ON);
      action = 1;
    }else{
      turnHeating(OFF);
      action = 0;
    }
    Serial.println("Action received: " + String(action));

    if(errorCount >= ERROR_COUNT_BEFORE_RESTART){
      Serial.println("\nDefined error count (" + String(ERROR_COUNT_BEFORE_RESTART) + ") reched!");
      doRestart();
    }
  }
  
}
 
void loop() {
  long now = millis();
  server.handleClient();
  wifiConnectionCheck(now);
  doOurTask(now);
  /**/
}
