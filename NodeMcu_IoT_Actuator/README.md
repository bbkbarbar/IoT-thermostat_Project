# IoT Actuator

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

Versions:
 - v0.1: basic functions
 - v0.2:
  - add "USE_MOCK" macro for using mock server instead of real thermostat
    for testing purpose - <br>
    <b>NOT WORKING with mock..</b>
  - getting action from real thermostat <b>WORKS FINE</b>


 TODO list:
  - Figure out WeMos board's pin configuration  
  - create functions for reading ADC <br>
    for observe heating phases of factory thermostat
  - Create circuite for "observe-function"
  - create methods for KaaIoT data upload (for observe function)
