/********************************************************************	\

  Name:         vme_reset.cxx
  Created by:   Stefano Piacentini

  Latest modification: 2026-07-08

  Contents: Frontend program for CYGNO-04

  Compile by:  g++ -o vme_reset vme_reset.cxx -lCAENVME

\********************************************************************/


#include <stdio.h>
#include "CAENVMElib.h"

int main() {
  int handle;
  CVErrorCodes ret;

  ret = CAENVME_Init(cvV1718, 0, 0, &handle);
  if(ret != cvSuccess) {
    printf("CAENVME_Init failed: %d\n", ret);
    return 1;
  }

  ret = CAENVME_SystemReset(handle);
  printf("CAENVME_SystemReset returned: %d\n", ret);

  CAENVME_End(handle);
  return 0;
}