#ifndef __TRGMOD_H__
#define __TRGMOD_H__

// B8:27:EB:EA:6A:D4

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstring>
#include <cctype>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "mqtt/client.h"
#include "mqttCustom.h"
//#include "midas.h"

class TRG
{
public:

  TRG(){
    LENGTH_REG = 8;
    SERVER_ADDRESS = "tcp://192.168.1.242:1883";
    
    QOS = 1;

    SETUP_TOPIC   = "trigger/setup";
    SERVICE_TOPIC = "trigger/service";
    CONF_TOPIC    = "trigger/conf";

    CLIENT_PUBLISH_ID = "sync_publish_cpp";
    CLIENT_CONSUME_ID = "paho_cpp_sync_consume";
    
    FPGAinit = false;
  }
  ~TRG(){}

  int trgmod_init(char *trgIP);

  int trgmod_config(int* trgreg);

  int trgmod_read();
  
  std::vector<int> getCONF() {
    return CONF;
  }

  std::string getServerAddress() {
    return SERVER_ADDRESS;
  }
  
private:
  int LENGTH_REG;
  std::string SERVER_ADDRESS;
  std::vector<int> CONF = {31, 31, 31, 3, 2, 0, 0, 0};
  int QOS;


  bool FPGAinit;

  // TOPICS FOR MQTT COMMUNICATION
  std::string SETUP_TOPIC;
  std::string SERVICE_TOPIC;
  std::string CONF_TOPIC;

  std::string CLIENT_PUBLISH_ID;
  std::string CLIENT_CONSUME_ID;
  
  int initializeFPGA(); 
  int configureFPGA(int *trgreg); 

  
};


#endif
