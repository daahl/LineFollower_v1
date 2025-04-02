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
uint8_t RemoteXY_CONF[] =   // 198 bytes
  { 255,10,0,2,0,191,0,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,12,0,7,61,52,40,10,118,64,2,26,2,67,61,25,
  40,10,86,2,26,7,61,79,40,10,118,64,2,26,2,7,61,105,40,10,
  110,64,2,26,2,5,129,4,107,27,8,64,17,80,118,97,108,117,101,0,
  129,4,26,40,8,64,17,84,117,114,110,86,97,108,117,101,0,129,4,54,
  52,8,64,17,66,87,84,104,114,101,115,104,111,108,100,0,129,4,79,25,
  8,64,17,83,112,101,101,100,0,129,4,133,55,8,64,17,83,97,118,101,
  32,69,69,80,82,79,77,0,129,4,168,41,8,64,17,66,101,101,112,98,
  101,101,112,33,0,2,67,131,30,11,0,2,26,31,31,83,97,118,101,0,
  0,10,69,159,24,24,48,4,1,31,79,78,0,31,79,70,70,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int16_t bwThreshold; // -32768 .. +32767
  int16_t speed; // -32768 .. +32767
  float Pvalue;
  uint8_t EEPROM; // =1 if switch ON and =0 if OFF
  uint8_t MotorState; // =1 if state is ON, else =0

    // output variables
  int16_t centerError; // -32768 .. +32767

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////