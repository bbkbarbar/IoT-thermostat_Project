// Used board setting:
// WeMos D1 R1


#define USE_SECRET  1
//#define USE_MOCK

// 1 "TP-Link_BB"    
// 2 "BB_Home2"  
// 3 "P20 Pro"    
// 4 "BB_guest"

//#define USE_TEST_CHANNEL
#define SKIP_TS_COMMUNICATION

#define VERSION                   "v1.4"
#define BUILDNUM                      20

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
#include "common_methods.h"


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
#define EEPROM_ADDR_MODE           8
#define EEPROM_ADDR_IP1            1
#define EEPROM_ADDR_IP2            2
#define EEPROM_ADDR_IP3            3
#define EEPROM_ADDR_IP4            4


//==================================
// "Data" variables
//==================================

short action = NOTHING; // can be NOTHING or HEATING

short modeSet = MODE_FORWARDER;
short modeToShow = modeSet;

//==================================
// Other variables
//==================================
long lastWiFiCheck = 0;
long lastCheck = 0;
int errorCount = 0;

unsigned long elapsedTime = 0;


// ==========================================================================================================================
//                                                     Hw function
// ==========================================================================================================================

String readFactoryInput(){
  //TODO
  int adc = analogRead(A0);
  if(adc > 680){
    Serial.println("Factory input: " + String(NEEDED_ACTION_HEATING) );
    return String(NEEDED_ACTION_HEATING);
  }
  if(adc < 340){
    Serial.println("Factory input: " + String(NEEDED_ACTION_NOTHING) );
    return String(NEEDED_ACTION_NOTHING);
  }
  Serial.println("Factory input: " + String(NEEDED_ACTION_UNKNOWN) );
  return String(NEEDED_ACTION_UNKNOWN);
}


// ==========================================================================================================================
//                                                     Other methods
// ==========================================================================================================================

void doRestart(){
  Serial.println("Restart now...");
  ESP.restart();
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
byte sendDataToKaaIoT(short retryCount, short heating){

    String heatingToSend = "";
    if(heating == 1){
      heatingToSend = "13";
    }else{
      heatingToSend = "0"; 
    }

    String postData = 
                "{\n";

    // NOT IMPORTANT
    postData += "\t\"RSSI-" + String(MODULE_NAME) + "\": " + String(WiFi.RSSI()) + ",\n";
    
    postData += "\t\"Actuator-heating\": "     + heatingToSend + "\n";
    postData += "}";

    Serial.println("PostData:\n" + postData);
    
    // new
    
    String url = String(KAA_POST_PATH);
    //String url = String(MOCK_SERVICE_PATH);

    //http.begin(client, url.c_str());  //Specify request destination
    WiFiClient wfc;
    http.begin(wfc, url);  //Specify request destination

    //http.addHeader("Content-Type", "text/plain");
    //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(postData);                                  //Send the request
    Serial.println("POST request sent: " + String(httpCode) + " " + url);

    //Close connection
    http.end();   //Close connection

    // TODO check http code if needed
    if( (httpCode != 200) && (retryCount > 0) ){
      sendDataToKaaIoT( (retryCount-1), heating);
    }

    return ((httpCode == 200)?1:0);
}
/**/


void HandleNotRstEndpoint(){
  doRestart();
}

void HandleRoot(){
  String message = "";
  message = getHTML(action, WiFi.RSSI(), modeToShow);
  /*
  message = generateHtmlHeader();
  message += generateHtmlBody();
  /**/
  server.send(200, "text/html", message );
}

void HandleGetFactoryControlState(){
  server.send(200, "text/html", readFactoryInput() );
  Serial.println("Factory control value sent..");
}

void actionModeSet(){
  String value = server.arg("mode"); //this lets you access a query param (http://x.x.x.x/set?value=12)
  short wantedMode = value.toInt();

  if( (wantedMode == 1) || (wantedMode == 7)){
    modeSet = MODE_ACTUATOR;
  }else{
    modeSet = MODE_FORWARDER;
  }
  
  // save into eeprom
  EEPROM.write(EEPROM_ADDR_MODE, modeSet);
  EEPROM.commit();

  
  String m = "Mode set to: " + (modeSet==MODE_FORWARDER?String("FORWARDER"):String("Actuator"));
  m += getHTMLAutomaticGoBack(3000);
  
  server.send(200, "text/html", m );/**/
}

String getNeededAction(){
    // old
    //http.begin("192.168.1.170:81/data");  //Specify request destination

    WiFiClient wc;
    
    String thermostatServerPath = "http://" + String(THERMOSTAT_IP) + "/result";
    http.begin(wc, thermostatServerPath.c_str());  //Specify request destination

    int httpCode = http.GET();                                  //Send the request
    
    String payload = "";
    Serial.println("httpCode: " + String(httpCode));
    if (httpCode == 200) { //Check the returning code
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
    
    // for reconnecting feature
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.hostname("IoT th " + String(MODULE_NAME) + " " + String(VERSION) + " b" + String(BUILDNUM));
  }else{
    Serial.println("");
    Serial.print("ERROR: Unable to connect to ");
    Serial.println(ssid);
    doRestart();
  }
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
//                                                        Init
// ==========================================================================================================================

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(POSITIVE_OUTPUT, OUTPUT);
  digitalWrite(POSITIVE_OUTPUT, HIGH);
  
  turnHeating(OFF);

  elapsedTime = millis();
  errorCount = 0;
  
  Serial.begin(SERIAL_BOUND_RATE);
  Serial.println();

  // EEPROM
  EEPROM.begin(128);
  modeSet = EEPROM.read(EEPROM_ADDR_MODE);
  /*
  String thermostatIP = String(EEPROM.read(EEPROM_ADDR_IP1))  + "."
                      + String(EEPROM.read(EEPROM_ADDR_IP2))  + "."
                      + String(EEPROM.read(EEPROM_ADDR_IP3))  + "."
                      + String(EEPROM.read(EEPROM_ADDR_IP4));
  /**/
  
  initWiFi();

  //Serial.println("Loaded IP: |" + thermostatIP + "|");

  //if(thermostatIP == "255.255.255.255")
//    thermostatIP = String(THERMOSTAT_IP);
  
  
  Serial.println("\n");
  Serial.println(String(SOFTWARE_NAME) + " "  + String(MODULE_NAME) + " " + String(VERSION) + " b" + String(BUILDNUM));
  #ifdef USE_MOCK
    Serial.println("MOCK USED!");
  #endif
  Serial.println("Thermostat IP: " + String(THERMOSTAT_IP));


  server.on("/", HandleRoot);
  server.on("/fc", HandleGetFactoryControlState);
  server.on("/set", actionModeSet);
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

long lastPublish = 0;

void doOurTask(long now){
  if( (now - lastCheck) > DELAY_BETWEEN_ITERATIONS_IN_MS ){ //Take a measurement at a fixed time (durationTemp = 5000ms, 5s)
    
    lastCheck = now;
    elapsedTime = now;

    String res = "";

    if(modeSet == MODE_FORWARDER){
      res = readFactoryInput();
      modeToShow = MODE_FORWARDER;
    }else{
      if( (modeSet == MODE_ACTUATOR) || (modeSet == MODE_ACTUATOR2) ){
        res = getNeededAction(); 
      } 
      if(res == String(NEEDED_ACTION_UNKNOWN)){
        modeToShow = MODE_ROLLBACK;
        res = readFactoryInput();
      }else{
        modeToShow = MODE_ACTUATOR;
      }
    }
    

    if( res == NEEDED_ACTION_HEATING){
      action = HEATING;  
      turnHeating(ON);
    }else{
      turnHeating(OFF);
      action = NOTHING;
    }
    Serial.println("Action to do: " + String(action));


    /*
    if(now - lastPublish > DELAY_BETWEEN_PUBLISHES_IN_MS){
      byte updateDone = sendDataToKaaIoT(1, action);
      if(updateDone){
        
      }
      lastPublish = now;
    }
    /**/

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
