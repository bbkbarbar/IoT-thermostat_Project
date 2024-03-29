
#define SOFTWARE_NAME  "IoT thermostat"
#define TITLE          "IoT TH sensor"
#define SHORT_NAME     "Thermostat"
#define MODULE_NAME    "UI"

#define LOCATION_NAME  "Nappali"
#define IOT_DEVICE_ID   2

#define PHASE_CHECKER_IP  "192.168.1.170"


// this is NOT equals to overheating different!
// this is only for temperature correction based on seen difference 
// between factory "Computherm" thermostat's dislpay and my DHT11 measurements

#define TEMPERATURE_CORRECTION   (-0.65f)

#define TEMPERATURE_MARGIN                 1    // 1 means 0.1°C


// =============================
// Pinout configuration
// =============================
 
#define DHT11_PIN                         14 // 14 means D5 in NodeMCU board -> Pin for temperature and humidity sensor

#define SOFT_SERIAL_RX                     5 //  5 means D1 in NodeMCU board
#define SOFT_SERIAL_TX                     4 //  4 means D2 in NodeMCU board


#define BTN_MINUS_ID                       1
#define BTN_PLUS_ID                        2

// =============================
// Wireles configuration
// =============================

// Server functions
#define SERVER_PORT                       80

// connect to WiFi
#define WIFI_TRY_COUNT                    50
#define WIFI_WAIT_IN_MS                 1000

#define PHASE_STATUS_RETRY_COUNT           1
#define KAAIOT_RETRY_COUNT                 1

// delay for normal working
#define DELAY_BETWEEN_ITERATIONS_IN_MS 30000

#define DELAY_BETWEEN_WIFI_CONNECTION_STATUS_CHECKS_IN_MS 20000

#define DELAY_BEFORE_REFRESH_PAGE_AFTER_SET_ENDPOINT_IN_MS 3000

// /set endpoint's arguments:
#define ARG_TEMP_SET1 "temp"
#define ARG_TEMP_SET2 "temperature"
#define ARG_OVERHEAT1 "overheat"
#define ARG_OVERHEAT2 "diff"

// problem handling
#define ERROR_COUNT_BEFORE_RESTART         3


// =============================
// logic constat definitions
// =============================

#define ON                                 1
#define OFF                                0

#define PHASE_STATUS_EXPENSIVE            "0"
#define PHASE_STATUS_CHEAP                "1"
#define PHASE_STATUS_UNKNOWN              "?"

#define KAA_VALUE_PHASE_STATUS_CHEAP       "1"
#define KAA_VALUE_PHASE_STATUS_UNKNOWN     "7"
#define KAA_VALUE_PHASE_STATUS_EXPENSIVE  "10"

#define NOTHING                             0
#define HEATING                             1

#define DISPLAY_NOT_HEATING               "_"
#define DISPLAY_HEATING                   "F"

#define FC_STATUS_NOTHING             "0"
#define FC_STATUS_HEATING             "1"
#define FC_STATUS_UNKNOWN             "?"
