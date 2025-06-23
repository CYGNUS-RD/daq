// console/misc/common.h
//
#include "console4.h"
#include <string>

void dcamcon_show_dcamerr( DCAMERR err, const char* apiname, const char* fmt=0, ...  );

HDCAM dcamcon_init_open(bool skipquestion = false);
HDCAM dcamcon_init_open_serial(const std::string& camserial);
void dcamcon_show_dcamdev_info( HDCAM hdcam );
void dcamcon_get_cameraid( HDCAM hdcam, std::string& cameraid);
