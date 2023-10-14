void sendDataToKaaIoT(short retryCount, String ps, float temperature , float humidity, float tempSet , float overheating , String heating, short fc){

    String phaseStatusToSend = "";
    if(ps == String(PHASE_STATUS_CHEAP)){
      phaseStatusToSend = String(KAA_VALUE_PHASE_STATUS_CHEAP);
    }else if(ps == String(PHASE_STATUS_EXPENSIVE)){
      phaseStatusToSend = String(KAA_VALUE_PHASE_STATUS_EXPENSIVE);
    }else{
      phaseStatusToSend = String(KAA_VALUE_PHASE_STATUS_UNKNOWN);
    }

    String heatingToSend = "";
    if(heating == "1"){
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