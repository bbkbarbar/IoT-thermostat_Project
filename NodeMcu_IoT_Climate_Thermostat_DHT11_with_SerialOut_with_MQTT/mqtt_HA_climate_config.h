char* configStr = "{\"curr_temp_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/ctt\", \"aux_cmd_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/act\"," "\"aux_stat_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/ast\"," "\"away_mode_cmd_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/amct\",  \"away_mode_stat_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/amst\", \"hold_cmd_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/hct\", \"hold_stat_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/hst\", \"min_temp\": 19, \"max_temp\": 27, \"temp_step\": 0.1, \"temp_cmd_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/ttct\", \"temp_stat_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/ttst\", \"mode_cmd_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/mct\", \"mode_stat_t\": \"homeassistant/climate/5002914e77b2/iot_thermostat/mst\", \"modes\": [   \"off\", \"auto\", \"heat\" ], \"name\": \"IoT Termosztát\", \"uniq_id\": \"iot_thermostat_5002914e77b2\", \"dev\": {   \"ids\": \"5002914e77b2\",   \"name\": \"IoT_Thermostat\",   \"sw\": \"1.0.0\" }, \"ret\": true }";

// Publish
#define TOPIC_MQTT_CLIMATE_CONFIG     "homeassistant/climate/5002914e77b2/iot_thermostat/config"
#define TOPIC_FEEDBACK_FOR_HA_TEMP_SET  "homeassistant/climate/5002914e77b2/iot_thermostat/ttst"
#define TOPIC_FEEDBACK_FOR_HA_MODE      "homeassistant/climate/5002914e77b2/iot_thermostat/mst"
#define TOPIC_FEEDBACK_CURR_TEMP      "homeassistant/climate/5002914e77b2/iot_thermostat/ctt"

// Subscribe
#define TOPIC_CMD_FROM_HA_TEMP_SET      "homeassistant/climate/5002914e77b2/iot_thermostat/ttct"
#define TOPIC_CMD_FROM_HA_MODE          "homeassistant/climate/5002914e77b2/iot_thermostat/mct"