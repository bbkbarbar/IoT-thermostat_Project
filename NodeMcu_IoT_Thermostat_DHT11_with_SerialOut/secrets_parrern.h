// =============================
// WiFi credentials
// =============================


#define SECRET_SSID_1                "SSID_1"    
#define SECRET_PASS_1                "pw1"    

#define SECRET_SSID_2                "SSID_2"  
#define SECRET_PASS_2                "SSID_2"   

#ifndef USE_SECRET
	#define USE_SECRET 1
#endif

#if USE_SECRET == 1
	#define S_SSID SECRET_SSID_1
	#define S_PASS SECRET_PASS_1
#elif USE_SECRET == 2
	#define S_SSID SECRET_SSID_2
	#define S_PASS SECRET_PASS_2
#else 
	#define S_SSID SECRET_SSID_4
	#define S_PASS SECRET_PASS_4
#endif




// =============================
// ThingSpeak connection details
// =============================
#ifdef USE_TEST_CHANNEL
  #define SECRET_CH_ID                    123456     
  #define SECRET_WRITE_APIKEY "ABCDEFGHIJKLMNO1"   
#else
  #define SECRET_CH_ID                    123457  
  #define SECRET_WRITE_APIKEY "ABCDEFGHIJKLMNO2"  
#endif
