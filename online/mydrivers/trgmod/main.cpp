#include "trgmod.h"


int main(int argc, char *argv[]) {

  TRG gTrg;

  char Ip[] = "192.168.1.242";
  
  gTrg.trgmod_init(Ip);

  int conf[8] = {31, 31, 31, 3, 2, 0, 0, 0};
  
  //gTrg.trgmod_config(conf);

  gTrg.trgmod_read();
  
  std::vector<int> out = gTrg.getCONF();

  for(unsigned int i = 0; i<out.size(); i++) {
    std::cout<<out[i]<<std::endl;
  }
  
  return 0;
}
