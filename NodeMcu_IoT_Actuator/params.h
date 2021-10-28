
#define SOFTWARE_NAME  "IoT thermostat"
#define TITLE          "IoT TH actuator"
#define MODULE_NAME    "actuator"

#define LOCATION_NAME  "Futes"
#define IOT_DEVICE_ID   3


#define THERMOSTAT_PROD_IP       "192.168.1.107"
#define THERMOSTAT_MOCK_IP  "192.168.1.143:8083"

#ifdef USE_MOCK
    #define THERMOSTAT_IP THERMOSTAT_MOCK_IP
#else
    #define THERMOSTAT_IP THERMOSTAT_PROD_IP
#endif

// =============================
// Pinout configuration
// =============================
 
#define DHT11_PIN                         14 // 14 means D5 in NodeMCU board -> Pin for temperature and humidity sensor

#define RELAY_PIN                         D2  // 2 means D4 in Wemos D1 Mini board
                                             // 5 means D1 in NodeMCU board
                                             // 17 means D4 in NodeMCU board


// =============================
// Wireles configuration
// =============================

// Server functions
#define SERVER_PORT                       80

// connect to WiFi
#define WIFI_TRY_COUNT                    50
#define WIFI_WAIT_IN_MS                 1000

// delay for normal working
#define DELAY_BETWEEN_ITERATIONS_IN_MS 10000

#define DELAY_BETWEEN_WIFI_CONNECTION_STATUS_CHECKS_IN_MS 20000

// problem handling
#define ERROR_COUNT_BEFORE_RESTART         3


// =============================
// logic constat definitions
// =============================

#define ON                                 1
#define OFF                                0

#define NOTHING                            0
#define HEATING                            1

#define NEEDED_ACTION_NOTHING            "0"
#define NEEDED_ACTION_HEATING            "1"
#define NEEDED_ACTION_UNKNOWN		   	 "?"
