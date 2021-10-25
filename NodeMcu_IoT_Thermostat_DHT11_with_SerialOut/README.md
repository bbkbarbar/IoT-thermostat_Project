# IoT Thermostat v2.5

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

TODO list:
 - Implement "/result" -endpoint for using by actuator device
 - tempSet can not be restored properly from EEPROM if it's last value > 255
   (overflow occures)
   Tried to store in 2 places and concatenate.. now working yet.