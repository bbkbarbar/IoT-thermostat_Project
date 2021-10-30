
# IoT Actuator


![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Circuit:
![image](https://github.com/bbkbarbar/IoT-thermostat_Project/blob/main/NodeMcu_IoT_Actuator/circuit.PNG?raw=true)

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
 - v0.3:
   - add relay handling
 - v1.0:
     - Can "forward" factory actuator's state
     - Can show factory actuator's state over "/fc" -endpoint


 TODO list:
  - Figure out WeMos board's pin configuration  
  - create functions for reading ADC <br>
    for observe heating phases of factory thermostat
  - Create circuite for "observe-function"
  - create methods for KaaIoT data upload (for observe function)
