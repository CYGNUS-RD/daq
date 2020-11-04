// console/misc/common.h
//
#include "console4.h"

void dcamcon_show_dcamerr( DCAMERR err, const char* apiname, const char* fmt=0, ...  );

HDCAM dcamcon_init_open();
void dcamcon_show_dcamdev_info( HDCAM hdcam );
