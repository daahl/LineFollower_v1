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
uint8_t RemoteXY_CONF[] =   // 96 bytes
  { 255,4,0,4,0,89,0,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,5,0,2,30,125,44,22,0,2,26,31,31,79,78,0,
  79,70,70,0,7,32,52,40,10,118,64,2,26,2,67,7,24,40,10,86,
  2,26,67,58,24,40,10,86,2,26,12,32,81,40,10,193,30,26,115,110,
  97,105,108,0,110,111,114,109,97,108,0,102,97,115,116,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t motorsState; // =1 if switch ON and =0 if OFF
  int16_t bwThreshold; // -32768 .. +32767
  uint8_t speedState; // from 0 to 3

    // output variables
  int16_t pixelValueLow; // -32768 .. +32767
  int16_t pixelValueHigh; // -32768 .. +32767

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////