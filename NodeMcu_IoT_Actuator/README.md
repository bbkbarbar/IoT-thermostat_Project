
# IoT Actuator


![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Circuit:
![image](https://github.com/bbkbarbar/IoT-thermostat_Project/blob/main/NodeMcu_IoT_Actuator/circuit.PNG?raw=true)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

 Endpoints:
  - / => Information page
  - /set => set mode (forwarder or actuator)
    - /set?mode=0  means "forwarder mode"
    - /set?mode=1  >> means "actuator mode"
  - /fc => factory thermostat's control state

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
 - v1.2:
     - /set endpoint added. <br>
     Mode can be set over it
 - v1.3:
     - Rename "Actuator" >> "Receiver"
     - WebUI changes (RSSI moved to "table")
     - Minor refactors
     - /set endpoint drop back to main page after few seconds
 - v1.4:
     - Rollback function:
       <br>If "Custom mode" used AND actuator can not get HTTP-200 -response from Thermostat device (for any reason)
       <br>than it automatically goes to "Rollback mode" and uses factory control state
     - v1.4.1:
       <br>In "Rollback mode" WebUI show the thermostat-IP


 TODO list:
  - ~~Add "Rollback" -mode: <br>
    Use "factory" input if thermostat not available for any reaseon~~
  - Consider create set..-endpoint for thermostat IP
  - ~~Figure out WeMos board's pin configuration~~
  - ~~create functions for reading ADC <br>
    for observe heating phases of factory thermostat~~
  - ~~Create circuite for "observe-function"~~
  - ~~create methods for KaaIoT data upload (for observe function)~~ - Solved since Thermostat module reads observed data from Receiver module
