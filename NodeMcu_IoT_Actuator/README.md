
# IoT Actuator


![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

Versions:
 - v0.1: basic functions
 - v0.2:
   - add "USE_MOCK" macro for using mock server instead of real thermostat
     for testing purpose
   - getting action from thermostat (mock or real) <b>WORKS FINE</b> b4
   - Using human-readable hostname b5
   - WebUI 2 (can shows mock-useage too) b9


 TODO list:
  - Figure out WeMos board's pin configuration  
  - create functions for reading ADC <br>
    for observe heating phases of factory thermostat
  - Create circuite for "observe-function"
  - create methods for KaaIoT data upload (for observe function)
