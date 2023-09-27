// =============================
// WiFi credentials
// =============================

#define SECRET_SSID_1                "TP-Link_BB"    
#define SECRET_PASS_1                "dand1katBalatonszemes"    

#define SECRET_SSID_2                "BB_Home2"  
#define SECRET_PASS_2                "hulyeamacska"   

#define SECRET_SSID_3                "P20 Pro"    
#define SECRET_PASS_3                "hulyeamacska"    

#define SECRET_SSID_4                "BB_guest"
#define SECRET_PASS_4                "vendeg01"

#ifndef USE_SECRET
  #define USE_SECRET 1
#endif

#if USE_SECRET == 1
  #define S_SSID SECRET_SSID_1
  #define S_PASS SECRET_PASS_1
#elif USE_SECRET == 2
  #define S_SSID SECRET_SSID_2
  #define S_PASS SECRET_PASS_2
#elif USE_SECRET == 3
  #define S_SSID SECRET_SSID_3
  #define S_PASS SECRET_PASS_3
#else 
  #define S_SSID SECRET_SSID_4
  #define S_PASS SECRET_PASS_4
#endif


// =============================
// MQTT connection details
// =============================

#define SECRET_MQTT_USER             "homeassistant"  
#define SECRET_MQTT_PASS             "Iephi1puhiexeiWiecie4aifooreramo5pooNgeem2ahvie7Thaibae1booxaech"  


// =============================
// KaaIoT connection details
// =============================
//#define KAA_POST_PATH "https://connect.cloud.kaaiot.com:443/kp1/c5r5udp04t2n6tdie500-v1/dcx/thermostat/json"
#define KAA_POST_PATH "http://connect.cloud.kaaiot.com/kp1/c5r5udp04t2n6tdie500-v1/dcx/thermostat/json"
#define MOCK_SERVICE_PATH "http://192.168.1.143:8083/post"

#define ACTUATOR_URL "http://192.168.1.172"


// =============================
// ThingSpeak connection details
// =============================
#ifdef USE_TEST_CHANNEL
  #define SECRET_CH_ID                    876121     
  #define SECRET_WRITE_APIKEY "1AH7O44C2M1ELPU8"   
#else
  #define SECRET_CH_ID                    691949  
  #define SECRET_WRITE_APIKEY "0IN79L3E7A1HC0AM"  
#endif
