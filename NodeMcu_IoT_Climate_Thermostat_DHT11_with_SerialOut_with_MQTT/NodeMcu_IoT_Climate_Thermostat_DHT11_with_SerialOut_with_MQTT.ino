
#define USE_SECRET  1

//#define USE_2_WAYS_COMMUNICATION
/* It means thermostat tries to get factory control state from receiver (if receiver can provide that) */

/*
 * Used board setting:
 * NodeMCU 1.0 (ESP-12E Module)
 */ 

//#define SHOW_OUTPUTS_FOR_DISLPAY_IN_SERIAL_TOO _


// 1 "TP-Link_BB"    
// 2 "BB_Home2"  
// 3 "P20 Pro"    
// 4 "BB_guest"

//#define USE_TEST_CHANNEL
#define SKIP_KAAIOT_SEND
#define SKIP_TS_COMMUNICATION

#define VERSION                  "v4.0.3"
#define BUILDNUM                      74

#define OWN_SENSOR                     0
#define EXTERNAL_SENSOR                1



/*
 * v3.0.7:
 *   Remove aux heating feature from climate device (since it become depricated in HASSOS 2023.09
 */


/*
 * Add device_name tag for RSSI
 */

#define SERIAL_BOUND_RATE         115200
#define SOFT_SERIAL_BOUND_RATE      9600

// DHT-11 module powered by NodeMCU's 5V 
// DHT-11 connected to D5 of NodeMCU
// Pinout: https://circuits4you.com/2017/12/31/nodemcu-pinout/

//==================================
// MQTT broker
//==================================
#define MQTT_HOST "192.168.1.190"
#define MQTT_PORT 1883

// Temperature MQTT Topic(s)
// _2 refers to IOT_DEVICE_ID
#define MQTT_PUB_TEMP "boorfeszek/iot_thermostat_2/temperature"
#define MQTT_PUB_HUM "boorfeszek/iot_thermostat_2/humidity"
#define MQTT_PUB_HEATING "boorfeszek/iot_thermostat_2/heating"
#define MQTT_PUB_TEMP_SET "boorfeszek/iot_thermostat_2/tempset"
#define MQTT_PUB_OVERHEAT "boorfeszek/iot_thermostat_2/overheat"
#define MQTT_PUB_OVERHEATVAL "boorfeszek/iot_thermostat_2/overheatval"
#define MQTT_PUB_RSSI "boorfeszek/iot_thermostat_2/rssi"
#define MQTT_PUB_DATA "boorfeszek/iot_thermostat_2/json"

#define MQTT_SUB_OHT "boorfeszek/iot_thermostat_2/setoverheat"

// MQTT Subscribe for additional features
#define TOPIC_TEMPERATURE_FROM_HA_SENSOR  "homeassistant/indoor_temp"
#define TOPIC_CHEAP_STATE_SENSOR          "boorfeszek/energy-cheap/state"

#include <SoftwareSerial.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>


#include <Ticker.h>
#include <AsyncMqttClient.h>


// KaaIoT
//#include <PubSubClient.h>
//#include <ArduinoJson.h>
//#include "kaa.h"

#include "secrets.h"
#include "params.h"

#include "mqtt_HA_climate_config.h"

#include "common_methods.h"
#include "webUI2.h"

DHTesp dht;

//==================================
// WIFI
//==================================
const char* ssid =           S_SSID;
const char* password =       S_PASS;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;


// =================================
// HTTP
// =================================
ESP8266WebServer server(SERVER_PORT);
HTTPClient http;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

//==================================
// KaaIoT
//==================================
WiFiClient client;


//==================================
// SoftSerial
//==================================
SoftwareSerial serialOut(SOFT_SERIAL_RX, SOFT_SERIAL_TX);      // (Rx, Tx)

//==================================
// Button handling 
//==================================
//Buttons hooked on D4 and D3 pins
#define BTN_PIN1                  13 // means D7 on NodeMCU board
#define BTN_PIN2                  15 // means D8 on NodeMCU board


//==================================
// EEPROM
//==================================
int eepromAddr =                   1;
int eepromAddr2 =                  4;

int usedSensor = OWN_SENSOR;


//==================================
// "Data" variables
//==================================
float valC = NAN;
float valF = NAN;
float valH = NAN;
float valT = NAN;
float valCExt = NAN;

#define AVG_ARR_SIZE 5
short dataCount = 0;
short nextDataIdx = 0;
float tempDataArr[AVG_ARR_SIZE];


short tempSet = 190; // means 19,0 °C
short overheatingDifference = 0;

short action = NOTHING; // can be NOTHING or HEATING
short currentMode = MODE_HEAT;

int lastTSUpdateStatus = -1;

long updateCount = 0;
String lastPhaseStatus = "";
String receivedPhaseState = "";

//==================================
// Other variables
//==================================
long timeOfLastMeasurement = 0;      //time of the last measurement
long lastWiFiCheck = 0;
int errorCount = 0;

unsigned long elapsedTime = 0;


boolean isValueInRange(float val, float max, float min, String text){

  if( (val != NAN) && (val <= max) && (val >= min) ){
    return true;
  }else{
    Serial.println("\n\n\nINVALID VALUE (" + text + "): " + String(val) + "\n\n");
    return false;
  }
}

boolean isValueSeemsReal(float val, String text){
  return isValueInRange(val, 100.0f, 2.0f, text);
}

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
  setTargetTemperature(value.toInt());
  float ts = (float)tempSet/10;
  Serial.println("Try to SET: " + String(tempSet) );
  String m = String("Target temperature value set:\n") + String(ts) + "C";
  m+= getHTMLAutomaticGoBack(DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS);

  //To force quick reaction of set comand
  timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
  server.send(200, "text/html", m );
}

void storeOverHeatValue(){
  // save into eeprom
  saveToEEPROM(eepromAddr2, overheatingDifference);
}

void actionOverheatSet(){
  String value = server.arg("value"); //this lets you access a query param (http://x.x.x.x/set?value=12)
  overheatingDifference = value.toInt();

  storeOverHeatValue();

  float oh = (float)overheatingDifference/10;
  Serial.println("Try to set overheating: " + String(overheatingDifference) );
  String m = String("Overheating value set: ") + String(oh)  + "C";
  m+= getHTMLAutomaticGoBack(DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS);

  //To force quick reaction of set comand
  timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
  server.send(200, "text/html", m );

  publishData(); // publish data as a standalone mqtt device..
}

void setTargetTemperature(int valueInt){
  // save into eeprom
  tempSet = valueInt;
  saveToEEPROM(eepromAddr, tempSet);

  Serial.println("Try to SET: " + String(tempSet) );
  //TODO: update display content

  //TODO: if MQTT is connected -> publish tempSet Value
  //if(mqttClient.isConnected())
    float tsF = (tempSet/10.0f);
    publishTempSet(tsF);
    //Serial.println("Publish done: |" + String(pl) + "|");

    if( (usedSensor == EXTERNAL_SENSOR) && (valCExt != NAN)){
      publishCurrentTemp(valCExt);
    }else{
      publishCurrentTemp(valC);
    }

    publishData(); // publish data as a standalone mqtt device..

    decide(); //MAYBE not needed
  
}

void actionSet(){
  String m = "";
  for (uint8_t i=0; i<server.args(); i++){
    if( (server.argName(i) == String(ARG_TEMP_SET1)) || (server.argName(i) == String(ARG_TEMP_SET2)) ){
        // TEMP_SET
        String value = server.arg(i);
        setTargetTemperature(value.toInt());

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

        publishData(); // publish data as a standalone mqtt device..

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
  if(lastPhaseStatus == String(PHASE_STATUS_CHEAP)){
    price = 0;
  }else{
    price = 1;
  }
  message = sendHTML(valC, ((float)(tempSet)/10), valH, price, action, WiFi.RSSI(), usedSensor, valCExt);
  /*
  message = generateHtmlHeader();
  message += generateHtmlBody();
  /**/
  server.send(200, "text/html", message );
}


String getCurrentPhaseState(int retryCount){

    if((receivedPhaseState != "") && (receivedPhaseState != PHASE_STATUS_UNKNOWN)){
      return receivedPhaseState;
    }
    else{

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
}



String getFactoryControlState(int retryCount){
    #ifdef USE_2_WAYS_COMMUNICATION
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
    #else
      return FC_STATUS_UNKNOWN;
    #endif
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


// #include "kaaIotFunctions.h"

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

//orig:
void connectToWifi(){
  // Setup WIFI
  WiFi.begin(S_SSID, S_PASS);
  
  Serial.println("MQTT server set to: " + String(MQTT_HOST));
  Serial.println("Try to connect (SSID: " + String(ssid) + ")");
  
  // Wait for WIFI connection
  int tryCount = 0;
  while ( (WiFi.status() != WL_CONNECTED) && (tryCount < WIFI_TRY_COUNT)) {
    delay(WIFI_WAIT_IN_MS);
    tryCount++;
    Serial.print(".");
  }
}

void initWiFi(){

  connectToWifi();
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("Connected");
    String ipMessage = "I" + WiFi.localIP().toString();
    serialOut.println(ipMessage);
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

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void publishCurrentTemp(float tempVal){

  if(isValueSeemsReal(tempVal, "temperature to publish") == false){
    return;
  }

  //Serial.println("\n\n\nStart publishing temperatureValue: " + String(tempVal) + "\n\n");
  String str = String(tempVal);
  int len = str.length();
  char* payload ="    ";
  str.toCharArray(payload, len);

  uint16_t packetIdPub1 = mqttClient.publish(TOPIC_FEEDBACK_CURR_TEMP, 1, true, payload);
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);

}

void publishMode(){
  if(currentMode == MODE_HEAT){
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC_FEEDBACK_FOR_HA_MODE, 1, true, "heat");
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
  } else
  if(currentMode == MODE_AUTO){
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC_FEEDBACK_FOR_HA_MODE, 1, true, "auto");
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
  } else
  if(currentMode == MODE_OFF){
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC_FEEDBACK_FOR_HA_MODE, 1, true, "off");
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
  }

}

void publishTempSet(float setVal){
  if(isValueSeemsReal(setVal, "tempSET to publish") == false){
    return;
  }
  String str = String(setVal);
  int len = str.length();
  char* payload ="    ";
  str.toCharArray(payload, len);

  uint16_t packetIdPub1 = mqttClient.publish(TOPIC_FEEDBACK_FOR_HA_TEMP_SET, 1, true, payload);
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  // Subscribe for needed topics:
  // Temp set
  uint16_t packetIdSub = mqttClient.subscribe(TOPIC_CMD_FROM_HA_TEMP_SET, 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);

  // Mode
  uint16_t packetIdSub2 = mqttClient.subscribe(TOPIC_CMD_FROM_HA_MODE, 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub2);

  // SetOverheat my HA via mqtt
  uint16_t packetIdSub3 = mqttClient.subscribe(MQTT_SUB_OHT, 2); // TODO: QoS is OK?
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub3);

  // Indoor temperature from external sensor(s)
  uint16_t packetIdSub4 = mqttClient.subscribe(TOPIC_TEMPERATURE_FROM_HA_SENSOR, 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub4);

  // Cheap state from external sensor over MQTT
  uint16_t packetIdSub5 = mqttClient.subscribe(TOPIC_CHEAP_STATE_SENSOR, 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub5);

  // Publish my config
  int configQoS = 1;
  mqttClient.publish(TOPIC_MQTT_CLIMATE_CONFIG, configQoS, true, configStr); //TODO payload
  Serial.println("Publishing at QoS 1");

  // Publish current temp
  //publishCurrentTemp(20.0f);

  // Publish temp set
  if(tempSet < 190){ // means 19.0C
    tempSet = 190;
  }
  float TsVal = (tempSet / 10.0f);
  publishTempSet(TsVal);

  // Publish current mode
  publishMode();

}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  //Serial.print("  payload: ");
  //Serial.println(payload);


  String topicStr = String(topic);
  String cmdTempSet = String(TOPIC_CMD_FROM_HA_TEMP_SET);

  if(topicStr == cmdTempSet){ // TempSet command

    String payloadStr = "";
    for(int i=0; i<total; i++){
      payloadStr += payload[i];
    }
    float targetTempFloat = payloadStr.toFloat();
    int tempToStore = targetTempFloat * 10;
    //Serial.println("    Uj homerseklet String: |" + payloadStr + "|");
    Serial.println("    Uj homerseklet Float:  |" + String(targetTempFloat) + "|");
    //Serial.println("    Uj homerseklet int:    |" + String(tempToStore) + "|");
    setTargetTemperature(tempToStore);
    updateDisplay();
  }

  else

  if(topicStr == String(TOPIC_CMD_FROM_HA_MODE)){
    String modeCmdStr = "";
    for(int i=0; i<total; i++){
      modeCmdStr += payload[i];
    }
    Serial.println("    Uj MODE str:  |" + String(modeCmdStr) + "|");
    int modeCmd = -1;
    if(modeCmdStr == String("auto")){
      modeCmd = MODE_AUTO;
      Serial.println("    Uj MODE cmd:  |MODE_AUTO|");
    }else
    if(modeCmdStr == String("heat")){
      modeCmd = MODE_HEAT;
      Serial.println("    Uj MODE cmd:  |MODE_HEAT|");
    }else
    if(modeCmdStr == String("off")){
      modeCmd = MODE_OFF;
      Serial.println("    Uj MODE cmd:  |MODE_OFF|");
    }


    if( (modeCmd == MODE_HEAT || modeCmd == MODE_AUTO) ){
      currentMode = MODE_HEAT;
      decide();
      publishMode();
    }else{ // cmd == OFF
      currentMode = MODE_OFF;
      publishMode();
    }
  }
  
  else

  if(topicStr == String(MQTT_SUB_OHT)){
    String payloadStr = "";
    for(int i=0; i<total; i++){
      payloadStr += payload[i];
    }
    Serial.println("Wanted overheat value: |" + payloadStr + "|\n\n");
    overheatingDifference = payloadStr.toInt();
    storeOverHeatValue();
    //To force quick reaction of set comand
    timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
    publishData(); // publish data as a standalone mqtt device..
  }

  else

  if(topicStr == String(TOPIC_TEMPERATURE_FROM_HA_SENSOR)){
    String payloadStr = "";
    for(int i=0; i<total; i++){
      payloadStr += payload[i];
    }
    Serial.println("Received external sensor value: |" + payloadStr + "|\n\n");
  
    valCExt = payloadStr.toFloat();

    //To force quick reaction of set comand
    timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
    usedSensor = EXTERNAL_SENSOR;
    publishData(); // publish data as a standalone mqtt device..
  }

  else

  if(topicStr == String(TOPIC_CHEAP_STATE_SENSOR)){
    String payloadStr = "";
    for(int i=0; i<total; i++){
      payloadStr += payload[i];
    }
    Serial.println("Received external cheap state: |" + payloadStr + "|\n\n");
    
    receivedPhaseState = payloadStr;
    //if( (receivedPhaseState != String(PHASE_STATUS_EXPENSIVE)) && (receivedPhaseState != String(PHASE_STATUS_CHEAP))){
    //  receivedPhaseState = String(PHASE_STATUS_UNKNOWN);
    //}

    //To force quick reaction of set comand
    timeOfLastMeasurement = timeOfLastMeasurement - DELAY_BETWEEN_ITERATIONS_IN_MS;
    publishData(); // publish data as a standalone mqtt device..
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

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials(SECRET_MQTT_USER, SECRET_MQTT_PASS);
  
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

  //Serial.println("short: " + String(sizeof(short)));
  //Serial.println("int: " + String(sizeof(int)));

  //setupButtons();
  
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

    /*
    int newMode = 9;
    if(currentMode == MODE_HEAT || currentMode == MODE_AUTO){
      if( lastPhaseStatus == String(PHASE_STATUS_CHEAP) ){
        newMode = MODE_AUTO;
      }else{
        newMode = MODE_HEAT;
      }
      if(newMode != currentMode){
        currentMode = newMode;
        publishMode();
      }
    }/**/
  
    if(action == HEATING){
      compareValue = compareValue + TEMPERATURE_MARGIN;
    }else{
      compareValue = compareValue - TEMPERATURE_MARGIN;
    }


    // TEMPERATURE COMPARE

    int v = 0;
    if( (usedSensor == EXTERNAL_SENSOR) && (valCExt != NAN)){
      v = ((int)( (valCExt+0.05)*10));
    }
    else{
      // USE OWN SENSOR
      v = ((int)( (valC+0.05)*10));
    }

    if( (valC != NAN) && (v < compareValue ) ){
      action = HEATING;
    }else{
      action = NOTHING;
    }
    

    //Serial.println("\n\nDECIDE:\nvalC: " + String(v) + "\ncompareValue: " + String(compareValue) + "\nACTION: "+ action + "\n\n");

    if(currentMode == MODE_OFF){
      action = NOTHING;
    }

    int newMode = 9;

    if(currentMode == MODE_HEAT || currentMode == MODE_AUTO){
      //Serial.println("\nAUTO || HEAT\n");
      if( action == HEATING ){
        newMode = MODE_HEAT;
      }else{
        newMode = MODE_AUTO;
      }

      if(newMode != currentMode){
        currentMode = newMode;
        publishMode();
      }
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

void updateDisplay(){
    int tempInt = valC*10;
    int humidityInt = valH;
    String line = getDisplayContent(lastPhaseStatus, tempInt, humidityInt, tempSet, action); 
    serialOut.println(line);
}

// Publish data as standAlone Mqtt device..
void publishData(){

      float ts = ((float)tempSet) / 10;
      float oh = (float)overheatingDifference/10;
      float ohv = ts+oh;
      int humInt = valH;
      int rounder = 0;
      if( (usedSensor == EXTERNAL_SENSOR) && (valCExt != NAN) ){
        rounder = (int)( (valCExt) * 10.0f);
      }else{
        rounder = (int)( (valC) * 10.0f);   // +0.05f handled in temperature_correction
      }
      
      float tempShow = (rounder / 10.0f);
      float currentlyUsedSetValue = (( lastPhaseStatus == String(PHASE_STATUS_CHEAP) )?(ohv):(ts) );
      String phaseStatusToPublish = (( lastPhaseStatus == String(PHASE_STATUS_CHEAP) )?(PHASE_STATUS_TO_BUBLISH_CHEAP):(PHASE_STATUS_TO_BUBLISH_EXPENSIVE) );

      // create Json content for publishing multiple data as 1 msg
      String jsonPayload = "{";
      if(isValueSeemsReal(tempShow, "json publish - temperature"))
        jsonPayload += String("\"temperature\": ") + String(tempShow).c_str() + String(", ");
      if(isValueInRange( (humInt*1.0f), 100.0f, MIN_CREDIBLE_HUMIDITY, "json publish - humidity"))
        jsonPayload += String("\"humidity\": ") + String(humInt).c_str() + String(", ");
      jsonPayload += String("\"temperature_set\": ") + String(ts).c_str() + String(", ");
      if(isValueInRange(oh, 10.0f, -10.0f, "json publish - overheating"))
        jsonPayload += String("\"overheating\": ") + String(oh).c_str() + String(", ");
      if(isValueSeemsReal(ohv, "json publish - overheated temp set"))
        jsonPayload += String("\"overheated_temp_set\": ") + String(ohv).c_str() + String(", ");
      if(isValueSeemsReal(currentlyUsedSetValue, "json publish - currently used temp set"))
        jsonPayload += String("\"currently_used_set_value\": ") + String(currentlyUsedSetValue).c_str() + String(", ");
      jsonPayload += String("\"heating\": ") + String(action).c_str() + String(", ");

      jsonPayload += String("\"considered_phase_state\": ") + phaseStatusToPublish.c_str() + String(", ");
      
      jsonPayload += String("\"rssi\": ") + String(WiFi.RSSI()).c_str();
      //jsonPayload += "\"name\": \"valueStr\", ";
      
      jsonPayload += String("}");

      Serial.println("\n"+ jsonPayload +"\n");

      // Publish MQTT msg with JSON payload
      uint16_t packetIdPub1Json = mqttClient.publish(MQTT_PUB_DATA, 1, true, jsonPayload.c_str());                            
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_DATA, packetIdPub1Json);
      Serial.println("Message: \n" + jsonPayload);
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
    //TODO: currently overwriting the measured value with received temperature value if received value is available
    // instead of this, we should simple use the received value, but in MQTT publish, we should use the external value for displaying in HA side..

    //if((usedSensor == EXTERNAL_SENSOR) && (valCExt != NAN) ){
    //  valC = valCExt;
    //}

    //valC = getTemperatureAvgValue(temperature + TEMPERATURE_CORRECTION);
    valH = humidity + HUMIDITY_CORRECTION;
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

    float tempShow = 0.0f;

    if((usedSensor == EXTERNAL_SENSOR) && (valCExt != NAN) ){
      int rounder = (int)( (valCExt) * 10.0f);   // +0.05f handled in temperature_correction
      tempShow = (rounder / 10.0f);
    }else{
      int rounder = (int)( (valC) * 10.0f);   // +0.05f handled in temperature_correction
      tempShow = (rounder / 10.0f);
    }

    publishCurrentTemp(tempShow);
    
    decide();

    timeOfLastMeasurement = now;

    //change
    updateDisplay();

    // change
    //delay(1500);
    delay(100);
    
    if(updateCounter == 1){
      #ifndef SKIP_KAAIOT_SEND
        //sendDataToKaaIoT(KAAIOT_RETRY_COUNT, lastPhaseStatus, valC, humidity, ts, oh, String(action), fcVal);
      #endif

      
      // Publish Measurement(s) as HA compatible climate device..
      

      publishData();

      // Publish MQTT msg - temperature
      int rounder = (int)( (valC) * 10.0f);   // +0.05f handled in temperature_correction
      float tempShow = (rounder / 10.0f);
      if(isValueSeemsReal(tempShow, "sandalone temperature")){
        uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(tempShow).c_str());                            
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub1);
        Serial.printf("Message: %.1f \n", valC);
      }

      // Publish MQTT msg - humidity
      if(isValueInRange( valH, 100.0f, MIN_CREDIBLE_HUMIDITY, "json publish - humidity")){
        uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(valH).c_str());                            
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_HUM, packetIdPub2);
        Serial.printf("Message: %.2f \n", valH);
      }

      // Publish MQTT msg - heating
      //uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_HEATING, 1, true, String(action).c_str());                            
      //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_HEATING, packetIdPub3);
      //Serial.printf("Message: %i \n", action);

      // Publish MQTT msg - tempset
      if(isValueSeemsReal(ts, "sandalone tempSet")){
        uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_TEMP_SET, 1, true, String(ts).c_str());                            
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP_SET, packetIdPub4);
        Serial.printf("Message: %.1f \n", ts);
      }

      // Publish MQTT msg - overheating
      //uint16_t packetIdPub5 = mqttClient.publish(MQTT_PUB_OVERHEAT, 1, true, String(oh).c_str());                            
      //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_OVERHEAT, packetIdPub5);
      //Serial.printf("Message: %.1f \n", oh);

      // Publish MQTT msg - overheating
      //uint16_t packetIdPub6 = mqttClient.publish(MQTT_PUB_OVERHEATVAL, 1, true, String(ohv).c_str());                            
      //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_OVERHEATVAL, packetIdPub6);
      //Serial.printf("Message: %.1f \n", ohv);
       
      // Publish MQTT msg - overheating
      //uint16_t packetIdPub7 = mqttClient.publish(MQTT_PUB_RSSI, 1, true, String(WiFi.RSSI()).c_str());                            
      //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_RSSI, packetIdPub7);
      //Serial.printf("Message: " + WiFi.RSSI());

    }

    updateCounter++;
    if(updateCounter > 3){
      updateCounter = 0;
    }
  
    if(errorCount >= ERROR_COUNT_BEFORE_RESTART){
      Serial.println("\nDefined error count (" + String(ERROR_COUNT_BEFORE_RESTART) + ") reached!");
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
