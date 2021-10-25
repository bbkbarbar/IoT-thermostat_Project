# NodeMcu IoT Thermostat DHT11 with SerialOut

TODO list:
 - tempSet can not be restored properly from EEPROM if it's last value > 255
   (overflow occures)
   Tried to store in 2 places and concatenate.. now working yet.