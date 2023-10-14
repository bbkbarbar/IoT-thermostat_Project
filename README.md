# IoT Thermostat v4.0.2

Custom made thermostat for building's heating control with capability of timing heat-phases when energy price is discounted.
It also has capability of different custom-interfaces (e.g.: show values on smart-mirror, etc.)

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)


### Modules:
 - #### [Thermostat](https://github.com/bbkbarbar/IoT-thermostat_Project/tree/main/NodeMcu_IoT_Thermostat_DHT11_with_SerialOut)
   - Measures inside temperature and humidity
   - Can use external temperature resource(s) e.g.: Any device (HomeAssistant or anything else) can provide temperature value for this thermostat over MQTT
     Due to lack of this "external" data it reverts to using own DTH sensor.
   - get price information from "PhaseChecker"-device
     - .. over MQTT
     - .. by using GET requests to "PhaseChecker"-device
   - Provide command to the "actuator" module (receiver) to control building's heater device
   - Shows informations on OLED display:
     - Current temperature
     - Set temperature
     - Humidity value (rh)
     - Current "price"
     - Current decision (heating or not)

 - #### [Actuator / Receiver](https://github.com/bbkbarbar/IoT-thermostat_Project/tree/main/NodeMcu_IoT_Actuator)
    - Can get action from thermostat module ("Custom mode")
    - Can get action from factory thermostat receiver ("factory mode")
    - Observe control state of factory thermostat receiver

 - #### PhaseChecker device:
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
