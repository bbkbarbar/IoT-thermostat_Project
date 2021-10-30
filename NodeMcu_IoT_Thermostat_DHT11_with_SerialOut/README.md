# IoT Thermostat v2.7

![image](https://raw.githubusercontent.com/bbkbarbar/IoT-thermostat_Project/main/IoT_Thermostat_v2.png)

Required libraries:
 - ESP8266 Community v2.7.3 (newer not applicable with currently used device(s))

Version notes:
- 2.7:
   - Add fuction to check factory control state over actuator device <br>
     and publish this information to KaaIoT
   - Change heeting-feedback from 1/0 to F/_
- 2.6:
   - Add hostname to WiFi client ✓
   - Send data to KaaIoT ✓ (b32)
   - WebUI 2 (b34) ✓
- 2.5:
   - Query phaseStatus infos from other device over WiFi ✓
   - Handle value settings from REST api calls on specified endpoints ✓
   - Show current data as webpage ✓
   - Serve a specified endpoint for actuator what contains only the needed action ✓



TODO list:
   - Create average value calculation(s) for KaaIoT update
   - Use "isnan()" -function
   - ~~Create recursive function for getCurrentPhaseState() with retryCount~~✓
   - ~~Create new webUI page (based on sample2.html)~~✓
   - ~~Implement color changes in heating's line in webUI 2~~ ✓
   - Store elapsed time since last KaaIoT update sent <br>
     for using during average calculations

   - ~~Data logging on local mock server should works fine~~ ✓
   - tempSet can not be restored properly from EEPROM if it's last value > 255
      (overflow occures)
    Tried to store in 2 places and concatenate.. now working yet.

Endpoints:
- /result => for checking wanted status by actuator
- /pure => return a JSON object what contains all available data:
   - IoT device id (int)
   - BuildNum (int)
   - location (name like "living room")
   - elapsedTime (long since device started)
   - tempC (float as string)
   - tempF (float as string)
   - humidity (float as string)
   - heatIndex (float as string)
   - targetTemp (float as string)
   - overheatingDiff (float as string)
   - phaseCheckerIP (as string)
   - phaseStatus (1|0)
   - heating (1|0)
- /data => show basic sensor values
- /data2 => show extended sensor values (+ phaseStatus)
- /set => set needed temperature
 "/set?value=235" (means 23,5°C)
- /overheat => set overheating temperature difference
 "/overheat?value=5" (means 0,5°C)
- /rst => reboot device insantly
