//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI_POINT

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "LineCam"
#define REMOTEXY_WIFI_PASSWORD "LineCam!"
#define REMOTEXY_SERVER_PORT 6377


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 363 bytes
  { 255,31,0,0,0,100,1,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,24,0,7,61,5,40,10,118,64,2,26,2,7,61,43,
  40,10,118,64,2,26,2,7,61,72,40,10,110,64,2,26,2,0,129,4,
  74,31,8,64,17,78,78,101,97,114,80,0,129,4,7,52,8,64,17,66,
  87,84,104,114,101,115,104,111,108,100,0,129,4,44,31,8,64,17,78,83,
  112,101,101,100,0,129,4,131,55,8,64,17,83,97,118,101,32,69,69,80,
  82,79,77,0,129,4,175,41,8,64,17,66,101,101,112,98,101,101,112,33,
  0,2,67,129,30,11,0,2,26,31,31,83,97,118,101,0,0,10,69,166,
  24,24,48,4,1,31,79,78,0,31,79,70,70,0,129,4,152,47,8,64,
  17,85,115,101,32,99,97,109,101,114,97,0,2,67,150,30,11,0,2,26,
  16,31,89,101,115,0,78,111,0,7,61,29,40,10,110,64,2,26,2,0,
  129,4,31,40,8,64,17,76,47,82,32,111,102,102,115,101,116,0,7,61,
  85,40,10,110,64,2,26,2,0,129,4,87,27,8,64,17,78,77,105,100,
  80,0,7,61,97,40,10,110,64,2,26,2,0,129,4,99,29,8,64,17,
  70,78,101,97,114,80,0,7,61,110,40,10,110,64,2,26,2,0,129,4,
  112,25,8,64,17,70,77,105,100,80,0,7,61,55,40,10,118,64,2,26,
  2,129,4,56,29,8,64,17,70,83,112,101,101,100,0,7,61,17,40,10,
  118,64,2,26,2,129,4,19,49,8,64,17,66,87,67,117,114,118,101,84,
  104,114,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int16_t bwThreshold; // -32768 .. +32767
  int16_t NSpeed; // -32768 .. +32767
  float NNearP;
  uint8_t EEPROM; // =1 if switch ON and =0 if OFF
  uint8_t MotorState; // =1 if state is ON, else =0
  uint8_t UseCamera; // =1 if switch ON and =0 if OFF
  float LROffset;
  float NMidP;
  float FNearP;
  float FMidP;
  int16_t FSpeed; // -32768 .. +32767
  int16_t BWCurveThr; // -32768 .. +32767

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////