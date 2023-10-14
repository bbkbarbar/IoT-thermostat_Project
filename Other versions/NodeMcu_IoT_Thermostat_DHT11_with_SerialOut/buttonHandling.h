byte lastStateOfBtn1 = 0;
byte lastStateOfBtn2 = 0;
long lastPressOfBtn1 = 0;
long lastPressOfBtn2 = 0;

void setupButtons(){
  pinMode(BTN_PIN1, INPUT);
  pinMode(BTN_PIN2, INPUT);
}

//void onBtnPressed(byte btnId);

void readButtonStates(){

  long now = millis();
  
    if(digitalRead(BTN_PIN1)){
      if(lastStateOfBtn1){
        // still pressed
      }else{
        if(now > (lastPressOfBtn1 + 50)){
          // Btn pressed now
          //onBtnPressed(1);
          lastStateOfBtn1 = 1;
          lastPressOfBtn1 = now;
          Serial.println("Btn 1 PRESSED!");
        }
      }
    }else{
      if(lastStateOfBtn1){
        // Btn released just now
        onBtnReleased(1);
        lastStateOfBtn1 = 0;
        Serial.println("Btn 1 released!");
      }else{
        // still NOT pressed
      }
  }

  if(digitalRead(BTN_PIN2) ){
      if(lastStateOfBtn2){
        // still pressed
      }else{
        if(now > (lastPressOfBtn2 + 50)){
          // Btn pressed now
          //onBtnPressed(2);
          lastStateOfBtn2 = 1;
          lastPressOfBtn2 = now;
          Serial.println("Btn 2 PRESSED!");
        }
      }
    }else{
      if(lastStateOfBtn2){
        // Btn released just now
        onBtnReleased(2);
        lastStateOfBtn2 = 0;
        Serial.println("Btn 2 released!");
      }else{
        // still NOT pressed
      }
  }
  
}
