# IoT Thermostat v2.6

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

Endpoints:
 - /result => for checking wanted status by actuator

Version notes:
 - 2.5:
    - Query phaseStatus infos from other device over WiFi
    - Handle value settings from REST api calls on specified endpoints
    - Show current data as webpage
    - Serve a specified endpoint for actuator what contains only the needed action
 - 2.6:
    - Send data to KaaIoT

TODO list:
 - Data logging on local mock server should works fine
 - tempSet can not be restored properly from EEPROM if it's last value > 255
   (overflow occures)
   Tried to store in 2 places and concatenate.. now working yet.