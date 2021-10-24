
#define TITLE          "IoT TH sensor"
#define SOFTWARE_NAME  "IoT thermostat"

#define LOCATION_NAME  "Nappali"
#define IOT_DEVICE_ID   1

#define PHASE_CHECKER_IP  "192.168.1.170"


// =============================
// Pinout configuration
// =============================
 
#define DHT11_PIN                         14 // 14 means D5 in NodeMCU board -> Pin for temperature and humidity sensor

#define SOFT_SERIAL_RX                     5 //  5 means D1 in NodeMCU board
#define SOFT_SERIAL_TX                     4 //  4 means D2 in NodeMCU board

// Server functions
#define SERVER_PORT                       80

// connect to WiFi
#define WIFI_TRY_COUNT                    50
#define WIFI_WAIT_IN_MS                 1000

// delay for normal working
#define DELAY_BETWEEN_ITERATIONS_IN_MS 30000

#define DELAY_BETWEEN_WIFI_CONNECTION_STATUS_CHECKS_IN_MS 20000

// problem handling
#define ERROR_COUNT_BEFORE_RESTART         3

#define PHASE_STATUS_EXPENSIVE           "0"
#define PHASE_STATUS_CHEAP               "1"
#define PHASE_STATUS_UNKNOWN             "?"

#define NOTHING                            0
#define HEATING                            1
