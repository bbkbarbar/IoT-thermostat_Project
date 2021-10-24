#define BUILD_NUM   4

/*
 *  Macro definitions for optional features
 */
#define USE_OLED_DISPLAY_SPI

//=================================================================================

#include "SSD1306Ascii.h"
#include <SoftwareSerial.h>
#include "SSD1306AsciiSoftSpi.h"

// pin definitions for using SPI oled display over softSpi
#define OLED_D0                                 2    
#define OLED_D1                                 3    
#define OLED_RST                                4    
#define OLED_DC                                 7    
#define OLED_CS                                 8    
#define SOFT_SERIAL_RX                          5
#define SOFT_SERIAL_TX                          6

#define SOFT_SERIAL_BOUND_RATE               9600
/**/

SSD1306AsciiSoftSpi oled;

SoftwareSerial s(SOFT_SERIAL_RX, SOFT_SERIAL_TX);      // (Rx, Tx)

//=================================================================================

void setup() {
  Serial.begin(9600);
  s.begin(SOFT_SERIAL_BOUND_RATE);

  oled.begin(&Adafruit128x64, OLED_CS, OLED_DC, OLED_D0, OLED_D1, OLED_RST);

  oled.setFont(Adafruit5x7);  
  oled.set2X();

  Serial.println("Thermostat OLED\Ndisplay started.");
  
  oled.clear();
  delay(200);
  oled.println("Display\nready...");

}

//=================================================================================

void show(String vez, int temp, int set, String heatingNow, String humS){
  String tempStr = String(temp/10) + "," + String(temp%10);
  String setStr = String(set/10) + "," + String(set%10);

  String energyType = "";
  if(vez == "0"){
    energyType = "N";
  }else if(vez == "1"){
    energyType = "_";
  }else{
    energyType = "?";
  }

  oled.clear();  
  
  oled.set2X();
  oled.println("A: "+ tempStr + " C");  // temp
  oled.set1X(); oled.println(""); oled.set2X();
  oled.println("S: "+ setStr + " C");  // setStr
  
  oled.set1X();
  oled.println(" ");
  oled.set2X();
  //oled.println("\nUUUUUUUUUUUUUUUUUU");
  oled.println(energyType + "  " + humS + "%  " + heatingNow);
}


#define P_HEATING       0
#define P_TEMP          1
#define P_SET           2
#define P_CONTROLLED    3

int i=0;
short hasNewInput = 0;

void loop(){
  
  String input = "";
  String strHeating =    "  ";
  String strTemp =   "     ";
  String strSet =    "     ";
  String strControlled = "  ";
  
  int currentPart = P_HEATING;
  int tempChar    = 0;

  //                01234567890123
  // Sample input: "D1-63%246C0"
  // Sample input: "I192.168.1.109"
  
  int ci = 0;
  input = "                      ";
  //input =   "D1-63%246C235C0"
  short len = 0;
  
  while (s.available()){
    char c = s.read();
    len++;
    hasNewInput = 1;
    
    input[ci] = c;
    ci++;
  }
  
  if(hasNewInput){
    hasNewInput = 0;
    
    if(input.substring(0,1) == "D"){
      //                    012345678901234
      String sampleInput = "D1-63%246C235C0";
    
      String phaseStatus = input.substring(1,2);
      String humidityNums = input.substring(3,5);
      String tempNums = input.substring(6,9);
      String tempSet = input.substring(10,13);
      String strHeating = input.substring(14,15);
      
      Serial.println(String(len) + " input: \"" + input + "\"");
      Serial.println("phaseStatus:  \""   + phaseStatus + "\"");
      Serial.println("tempNums:     \""   + tempNums + "\"");
      Serial.println("tempSet:      \""   + tempSet + "\"");
      Serial.println("humidityNums: \""   + humidityNums + "\"");
      Serial.println("strHeating: \""    + strHeating + "\"\n");
      //Serial.println("strTemp: "       + strTemp);
      //Serial.println("strSet: "        + strSet);
      //Serial.println("strControlled: " + strControlled);
  
      show(phaseStatus, tempNums.toInt(), tempSet.toInt(), strHeating, humidityNums);
    }else
    if(input.substring(0,1) == "I"){
      // Sample input: "I192.168.1.109"
      oled.clear();  
      oled.set2X();
      oled.println("IP:");
      oled.set1X();
      oled.println(input.substring(1,8) + "\n"); 
      oled.set2X();
      oled.println(input.substring(8));  
    }
    //oled.clear();
    //oled.println(input);
  }
  
  delay(1000);

}
