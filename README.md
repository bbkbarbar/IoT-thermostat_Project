# IoT Thermostat v2.7

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)


### Modules:
 - Thermostat
  - Measures inside temperature and humidity
  - get price information from "PhaseChecker"-device
  - Provide command to the "actuator" module (receiver) to control building's heater device
  - Shows informations on OLED display:
    - Current temperature
    - Set temperature
    - Humidity value (rh)
    - Current "price"
    - Current decision (heating or not)

 - Actuator / Receiver:
  - Can get action from thermostat module ("Custom mode")
  - Can get action from factory thermostat receiver ("factory mode")
  - Observe control state of factory thermostat receiver

 - PhaseChecker device:
   - Provide current status of "low-priced" energy source (currently available or not)

#### Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

#### Hardware requirement:
- Thermostat
 - NodeMCU board (or WeMos D1 mini also applicable)
 - DHT11 temperature and humidity sensor (DHT22 also applicable)
 - Arduino Nano (other arduino version can also be used)
 - OLED display (0.96" with SPI interface)

- Actuator
 - WeMos D1 mini board (NodeMCU also applicable)
 - Relay module (1ch, low level triggered version)
 - NPN transistor and resistors for control relay-module

ToDo list:
