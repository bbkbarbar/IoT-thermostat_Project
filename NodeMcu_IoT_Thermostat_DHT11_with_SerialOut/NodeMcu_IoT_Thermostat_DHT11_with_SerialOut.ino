
#define USE_SECRET  1

/*
 * Used board setting:
 * NodeMCU 1.0 (ESP-12E Module)
 */ 

//#define SHOW_OUTPUTS_FOR_DISLPAY_IN_SERIAL_TOO

//#define SKIP_KAAIOT_SEND

// 1 "TP-Link_BB"    
// 2 "BB_Home2"  
// 3 "P20 Pro"    
// 4 "BB_guest"

//#define USE_TEST_CHANNEL
#define SKIP_TS_COMMUNICATION

#define VERSION                   "v2.8.1"
#define BUILDNUM                      41
/*
 * Add device_name tag for RSSI
 */

#define SERIAL_BOUND_RATE         115200
#define SOFT_SERIAL_BOUND_RATE      9600

// DHT-11 module powered by NodeMCU's 5V 
// DHT-11 connected to D5 of NodeMCU
// Pinout: https://circuits4you.com/2017/12/31/nodemcu-pinout/

#include <SoftwareSerial.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

// KaaIoT
//#include <PubSubClient.h>
//#include <ArduinoJson.h>
//#include "kaa.h"

#include "secrets.h"
#include "params.h"

#include "common_methods.h"
#include "webUI2.h"

DHTesp dht;

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
// KaaIoT
//==================================
WiFiClient client;


//==================================
// SoftSerial
//==================================
SoftwareSerial serialOut(SOFT_SERIAL_RX, SOFT_SERIAL_TX);      // (Rx, Tx)

//==================================
// EEPROM
//==================================
int eepromAddr =                   1;
int eepromAddr2 =                  4;


//==================================
// "Data" variables
//==================================
float valC = NAN;
float valF = NAN;
float valH = NAN;
float valT = NAN;

#define AVG_ARR_SIZE 5
short dataCount = 0;
short nextDataIdx = 0;
float tempDataArr[AVG_ARR_SIZE];


short tempSet = 150; // means 15,0 °C
short overheatingDifference = 0;

short action = NOTHING; // can be NOTHING or HEATING

int lastTSUpdateStatus = -1;

long updateCount = 0;
String lastPhaseStatus = "";

//==================================
// Other variables
//==================================
long timeOfLastMeasurement = 0;      //time of the last measurement
long lastWiFiCheck = 0;
int errorCount = 0;

unsigned long elapsedTime = 0;


float getTemperatureAvgValue(float lastVal){

  // store last value
  tempDataArr[nextDataIdx] = lastVal; 
  
  //move "pointer" of next overrideable array element
  nextDataIdx++;
  if(nextDataIdx >= AVG_ARR_SIZE){
    nextDataIdx = 0;
  }
  

  float res = 0.0;
  
  if(dataCount < AVG_ARR_SIZE){ // we have no enaugh data for calculation2, so we will calculate a basic average of already available values..
    dataCount++;
    for (uint8_t i=0; i<dataCount; i++){
      res += tempDataArr[i];
    }
    return (res / dataCount);
  
  }else{ // we have full array of data

    byte idxMin = 0;
    byte idxMax = 0;
    float minV = tempDataArr[idxMin];
    float maxV = tempDataArr[idxMax];
    for (uint8_t i=1; i<dataCount; i++){
      if(tempDataArr[i] < tempDataArr[idxMin]){
        idxMin = i;
      }
      if(tempDataArr[i] > tempDataArr[idxMax]){
        idxMax = i;
      }
    }

    if(idxMin == idxMax){ // means all element of array is the same value
      // than return with any element of array e.g.: idx0
      return tempDataArr[0];
    }

    // if we reached this line of code than we have an array with different min and max values..
    for (uint8_t i=0; i<AVG_ARR_SIZE; i++){
      // we will get a total value of array elements except the min and max value
      if((i != idxMin) && (i != idxMax)){
        res += tempDataArr[i];  
      }
    }
    return (res / (AVG_ARR_SIZE-2));
    
  }

}


// ==========================================================================================================================
//                                                 KaaIOT communication
// ==========================================================================================================================



// ==========================================================================================================================
//                                                 Hw feedback function
// ==========================================================================================================================

void turnLED(int state){
  if(state > 0){
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }else{
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void ledBlink(int blinkCount, int onTime, int offTime){
  for(short int i=0; i<blinkCount; i++){
    turnLED(ON);
    delay(onTime);                      // Wait for a second
    turnLED(OFF);
    if(i < blinkCount-1){
      delay(offTime);
    }
  }
}

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


void actionTempSet(){
  String value = server.arg("value"); //this lets you access a query param (http://x.x.x.x/set?value=12)
  tempSet = value.toInt();

  // save into eeprom
  saveToEEPROM(eepromAddr, tempSet);
  float ts = (float)tempSet/10;
  Serial.println("Try to SET: " + String(tempSet) );
  String m = String("Target temperature value set:\n") + String(ts) + "C";
  m+= getHTMLAutomaticGoBack(DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS);

  //To force quick reaction of set comand
  timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
  server.send(200, "text/html", m );
}


void actionOverheatSet(){
  String value = server.arg("value"); //this lets you access a query param (http://x.x.x.x/set?value=12)
  overheatingDifference = value.toInt();

  // save into eeprom
  saveToEEPROM(eepromAddr2, overheatingDifference);

  float oh = (float)overheatingDifference/10;
  Serial.println("Try to set overheating: " + String(overheatingDifference) );
  String m = String("Overheating value set: ") + String(oh)  + "C";
  m+= getHTMLAutomaticGoBack(DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS);

  //To force quick reaction of set comand
  timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
  server.send(200, "text/html", m );
}


void actionSet(){
  String m = "";
  for (uint8_t i=0; i<server.args(); i++){
    if( (server.argName(i) == String(ARG_TEMP_SET1)) || (server.argName(i) == String(ARG_TEMP_SET2)) ){
        // TEMP_SET
        String value = server.arg(i);
        tempSet = value.toInt();

        // save into eeprom
        saveToEEPROM(eepromAddr, tempSet);

        Serial.println("Try to SET: " + String(tempSet) );
        float ts = (float)tempSet/10;
        m += String("<br>Target temperature value set:\n") + String(ts) + "C\n<br>";

        //To force quick reaction of set comand
        timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
    }
    if( (server.argName(i) == String(ARG_OVERHEAT1)) || (server.argName(i) == String(ARG_OVERHEAT2)) ){
        // OVERHEAT
        String value = server.arg(i);
        overheatingDifference = value.toInt();

        // save into eeprom
        saveToEEPROM(eepromAddr2, overheatingDifference);

        Serial.println("Try to set overheating: " + String(overheatingDifference) );
        float oh = (float)overheatingDifference/10;
        m += String("Overheating value set: ") + String(oh)  + "C\n<br>";

        //To force quick reaction of set comand
        timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
    }
  }
  
  m+= getHTMLAutomaticGoBack(DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS);
  server.send(200, "text/html", m );
}



void HandleNotRstEndpoint(){
  doRestart();
}



void HandleRoot(){
  //Serial.println("HandleRoot called...");
  //String message = "<!DOCTYPE html><html lang=\"en\">";
  //message += "<head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>";
  String message = "";
  short price = 0;
  if(lastPhaseStatus == String(PHASE_STATUS_EXPENSIVE)){
    price = 0;
  }else{
    price = 1;
  }
  message = sendHTML(valC, ((float)(tempSet)/10), valH, price, action);
  /*
  message = generateHtmlHeader();
  message += generateHtmlBody();
  /**/
  server.send(200, "text/html", message );
}


String getCurrentPhaseState(int retryCount){
     WiFiClient myWC;
    // old
    //http.begin("192.168.1.170:81/data");  //Specify request destination
    // new
    String statusServerPath = "http://" + String(PHASE_CHECKER_IP) + ":81/data";
    http.begin(myWC, statusServerPath.c_str());  //Specify request destination


    int httpCode = http.GET();                                  //Send the request
    //Serial.print("PhaseStatus - resultCode: " + String(httpCode) + " ps: ");
    
    String payload = "";
    if (httpCode > 0) { //Check the returning code
      payload = http.getString();   //Get the request response payload
      //Serial.print(payload);
      //Serial.println(payload);             //Print the response payload
    }else{
      payload = String(PHASE_STATUS_UNKNOWN);
      Serial.println("httpCode: " + String(httpCode));
    }
    http.end();   //Close connection

    if( (retryCount > 0) && (payload == String(PHASE_STATUS_UNKNOWN)) ){
      // probably need to retry
      return getCurrentPhaseState(--retryCount);
    }else{
      return payload;
    }
}


String getFactoryControlState(int retryCount){
     WiFiClient myWC;
    // old
    //http.begin("192.168.1.170:81/data");  //Specify request destination
    // new
    String statusServerPath = String(ACTUATOR_URL) + "/fc";
    http.begin(myWC, statusServerPath.c_str());  //Specify request destination


    int httpCode = http.GET();                                  //Send the request
    //Serial.print("PhaseStatus - resultCode: " + String(httpCode) + " ps: ");
    
    String payload = "";
    if (httpCode > 0) { //Check the returning code
      payload = http.getString();   //Get the request response payload
      //Serial.print(payload);
      //Serial.println(payload);             //Print the response payload
    }else{
      payload = String(FC_STATUS_UNKNOWN);
      Serial.println("httpCode: " + String(httpCode));
    }
    http.end();   //Close connection

    Serial.println("FactoryState: " + payload);

    if( (retryCount > 0) && (payload == String(FC_STATUS_UNKNOWN)) ){
      // probably need to retry
      return getFactoryControlState(--retryCount);
    }else{
      return payload;
    }
}


#ifdef DEBUG_MODE
void sendLogs(String lastPhaseStatus, String temperature , String ts , String oh , String heating){
     //Declare an object of class HTTPClient

    String getData = "?ps=" + lastPhaseStatus + "&temp=" + temperature + "&tempSet=" + ts + "&overheat=" + oh + "&heating=" + heating;
    
    // new
    String logServerPath = "http://192.168.1.141:8083/datalogger2" + getData;
    http.begin(client, logServerPath.c_str());  //Specify request destination

    http.addHeader("Content-Type", "text/plain");
    int httpCode = http.GET();                                  //Send the request

    Serial.println("Logs sent: " + getData);

    //Close connection
    http.end();   //Close connection
}
#endif


void sendDataToKaaIoT(short retryCount, String ps, float temperature , float humidity, float tempSet , float overheating , String heating, short fc){

    String phaseStatusToSend = "";
    if(ps == "1"){
      phaseStatusToSend = "1";
    }else{
      phaseStatusToSend = "10"; 
    }

    String heatingToSend = "";
    if(heating == String(HEATING)){
      heatingToSend = "12";
    }else{
      heatingToSend = "0"; 
    }

    float ct = tempSet + overheating;
  
    String postData = 
                "{\n";
    postData += "\t\"temperature\": " + String(temperature) + ",\n";
    postData += "\t\"tempSet\": "     + String(tempSet) + ",\n";
    postData += "\t\"calculatedTarget\": " + String(ct) + ",\n";
    postData += "\t\"humidity\": "    + String(humidity) + ",\n";
    postData += "\t\"overheating\": " + String(overheating) + ",\n";
    postData += "\t\"factoryControlState\": " + String(fc) + ",\n";

    // NOT IMPORTANT
    postData += "\t\"RSSI-" + String(SHORT_NAME) + "\": " + String(WiFi.RSSI()) + ",\n";
    
    postData += "\t\"phaseStatus\": " + phaseStatusToSend + ",\n";
    postData += "\t\"heating\": "     + heatingToSend + "\n";
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
      sendDataToKaaIoT( (retryCount-1), ps, temperature, humidity, tempSet, overheating, heating, fc);
    }

}


void GetNeededAction(){
  String message = "" + String(action);
  server.send(200, "text/html", message );
}

void HandleData(){
  //Serial.println("HandleData called...");
  String message = "" +  String(valC) + " " +  String(valH);
  server.send(200, "text/html", message );
}

void HandleMultipleData(){
  //Serial.println("HandleData called...");
  String message = "" +  String(valC) + "|" +  String(valH) + "|" + lastPhaseStatus;
  server.send(200, "text/html", message );
}


void showPureValues(){
  Serial.println("showPureValues() called...");
  float ts = ((float)tempSet) / 10;
  float ohd = ((float)overheatingDifference) / 10;
  String message = "{\n";
  
  message += "\t\"IoT device id\": \"" + String(IOT_DEVICE_ID) + "\",\n";
  message += "\t\"BuildNum\": \"" + String(BUILDNUM) + "\",\n";
  message += "\t\"location\": \"" + String(LOCATION_NAME) + "\",\n";
  message += "\t\"elapsedTime\": \"" + String(elapsedTime) + "\",\n";
  message += "\t\"values\": {\n";
  message += "\t\t\"tempC\": \"" + String(valC) + "\",\n";
  message += "\t\t\"tempF\": \"" + String(valF) + "\",\n";
  message += "\t\t\"humidity\": \"" + String(valH) + "\",\n";
  message += "\t\t\"heatIndex\": \"" + String(valT) + "\"\n";
  message += "\t},\n";
  message += "\t\"parameters\": {\n";
  message += "\t\t\"targetTemp\": \"" + String(ts) + "\",\n";
  message += "\t\t\"overheatingDiff\": \"" + String(ohd) + "\"\n";
  message += "\t},\n";
  message += "\t\"phaseChecker\": {\n";
  message += "\t\t\"phaseCheckerIP\": \"" + String(PHASE_CHECKER_IP) + "\",\n";
  message += "\t\t\"phaseStatus\": \"" + lastPhaseStatus + "\"\n";
  message += "\t},\n";
  message += "\t\"heating\": \"" + String(action) + "\"\n";
  message += "}\n";
  server.send(200, "text/json", message );
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
    serialOut.println(ipMessage);
    //Serial.println("SENT: " + ipMessage );
    
    // for reconnecting feature
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.hostname(String(SOFTWARE_NAME) + " " + String(MODULE_NAME) + " [" + String(IOT_DEVICE_ID) + "] " + String(VERSION) + " b" + String(BUILDNUM));
  }else{
    Serial.println("");
    Serial.print("ERROR: Unable to connect to ");
    Serial.println(ssid);
    doRestart();
  }
}


void wifiConnectionCheck(long now){
  //TODO: need to add optional macro for Serial prints of WiFi state
  if( (now - lastWiFiCheck) >= DELAY_BETWEEN_WIFI_CONNECTION_STATUS_CHECKS_IN_MS ){
    lastWiFiCheck = now;
    
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL:
        Serial.println("Configured SSID cannot be reached");
        serialOut.println("IWiFi\nUnavailable");
        turnLED(ON);
        break;
      case WL_CONNECTED:
        turnLED(OFF);
        //Serial.println("Connection successfully established");
        break;
      case WL_CONNECT_FAILED:
        turnLED(ON);
        Serial.println("Connection failed");
        serialOut.println("IConnection\nfailed");
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

// ==========================================================================================================================
//                                                        Init
// ==========================================================================================================================
 
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  elapsedTime = millis();
  
  turnLED(ON);

  errorCount = 0;
  
  Serial.begin(SERIAL_BOUND_RATE);
  Serial.println();
  Serial.println("\n");
  Serial.print(String(TITLE) + " ");
  Serial.println(VERSION);

  serialOut.begin(SOFT_SERIAL_BOUND_RATE);
  Serial.println("SoftSerial initialized");

  // EEPROM
  EEPROM.begin(128);
  // EEPROM read methods can not be used before the first write method..
  tempSet = loadFromEEPROM(eepromAddr);
  Serial.println("Set temperature: " + String(tempSet));
  overheatingDifference = loadFromEEPROM(eepromAddr2);
  Serial.println("Overheating difference: " + String(overheatingDifference));

  
  dht.setup(DHT11_PIN, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  //dht.setup(DHTpin, DHTesp::DHT22); //for DHT22 Connect DHT sensor to GPIO 17

  initWiFi();


  server.on("/", HandleRoot);
  server.on("/result", GetNeededAction);
  server.on("/data", HandleData);
  server.on("/data2", HandleMultipleData);
  //server.on ("/save", handleSave);
  server.on("/pure", showPureValues);
  server.on("/rst", HandleNotRstEndpoint);
  server.on("/set", actionSet);
  server.on("/temp", actionTempSet);
  server.on("/overheat", actionOverheatSet);
  server.onNotFound( HandleNotFound );
  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString() + String("@ port: ") + String(SERVER_PORT) );

  Serial.println("short: " + String(sizeof(short)));
  Serial.println("int: " + String(sizeof(int)));
  
  turnLED(OFF);

  Serial.println("PhaseStatus\tStatus\tHumidity (%)\tTemperature (C)\t\tTempSet\tOverheating (.C)\t(F)\tHeatIndex (C)\t(F)");
  timeOfLastMeasurement = -100000;

  delay(10000);
  
}


// ==========================================================================================================================
//                                                        Loop
// ==========================================================================================================================


short decide(){

  int compareValue = tempSet;
  if( lastPhaseStatus == String(PHASE_STATUS_CHEAP) ){
    compareValue += overheatingDifference; // TODO
  }

  if(action == NOTHING){
    compareValue = compareValue - TEMPERATURE_MARGIN;
  }
  if(action == HEATING){
    compareValue = compareValue + TEMPERATURE_MARGIN;
  }
  
  if( (valC != NAN) && (((int)( (valC+0.05)*10)) < compareValue ) ){
      action = HEATING;
    }else{
      action = NOTHING;
    }
  
  return action;
}

int updateCounter = 0;

String getDisplayContent(String phaseStatusStr, int tempInt, int humidityInt, int ts, int heatingAction ){
    String line = "D" + phaseStatusStr;
    String tempStr = String(tempInt);
    if(tempInt < 100){
      tempStr = "0" + tempStr;
    }
    String setStr = String(ts);
    
    if(ts < 100){
      setStr = "0" + setStr;
    }
    if(ts < 10){
      setStr = "0" + setStr;
    }
    line += "-" + String(humidityInt) + "%";
    line += tempStr + "C";
    line += setStr + "C";
    // "heating?" part
    line += (heatingAction==1?String(DISPLAY_HEATING):String(DISPLAY_NOT_HEATING));
  
    #ifdef SHOW_OUTPUTS_FOR_DISLPAY_IN_SERIAL_TOO
      Serial.println("\"" + line + "\"");
    #endif
  
    return line;
}


void sensorLoop(long now){
  if( (now - timeOfLastMeasurement) > DELAY_BETWEEN_ITERATIONS_IN_MS ){ //Take a measurement at a fixed time (durationTemp = 5000ms, 5s)
    //delay(dht.getMinimumSamplingPeriod());

    
    lastPhaseStatus = getCurrentPhaseState(PHASE_STATUS_RETRY_COUNT);
    
    elapsedTime = now;
  
    turnLED(ON);
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    float ts = ((float)tempSet) / 10;
    float oh = (float)overheatingDifference/10;
    turnLED(OFF);


    valC = temperature + TEMPERATURE_CORRECTION;
    //valC = getTemperatureAvgValue(temperature + TEMPERATURE_CORRECTION);
    valH = humidity;
    valF = dht.toFahrenheit(temperature);
    valT = dht.computeHeatIndex(temperature, humidity, false);
   
    Serial.print("\t" + lastPhaseStatus);
    Serial.print("\t");
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("% \t\t");
    
    Serial.print(valC, 2);
    Serial.print(" C\t|  ");
    valC = getTemperatureAvgValue(valC);
    Serial.print(valC, 2);
    Serial.print(" C\t");
    
    Serial.print(ts, 1);
    Serial.print(" C\t");
    Serial.print("\t" + String(oh) + " C\t");
    Serial.print(valF, 1);
    Serial.print(" F\t");
    Serial.print(valT, 1);
    Serial.print("\t");
    Serial.print(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
    Serial.print("\t");
    Serial.println(String(WiFi.RSSI()));

    String fc = getFactoryControlState(1);
    short fcVal = 5;
    if(fc == String(FC_STATUS_HEATING)){
      fcVal = 9;
    }else if(fc == String(FC_STATUS_NOTHING)){
      fcVal = 0;
    }else{
      fcVal = 5;
    }

    decide();

    timeOfLastMeasurement = now;
    int tempInt = valC*10;
    int humidityInt = valH;
    String line = getDisplayContent(lastPhaseStatus, tempInt, humidityInt, tempSet, action); 
    serialOut.println(line);


    delay(1500);
    if(updateCounter == 1){
      #ifndef SKIP_KAAIOT_SEND
      
        sendDataToKaaIoT(KAAIOT_RETRY_COUNT, lastPhaseStatus, valC, humidity, ts, oh, String(action), fcVal);
        
      #endif
    }
    updateCounter++;
    if(updateCounter > 4){
      updateCounter = 0;
    }
  
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
  sensorLoop(now);
}
