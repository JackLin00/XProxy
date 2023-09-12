#ifndef __USER_LOG_H__
#define __USER_LOG_H__

#include "hv/hlog.h"



#define DEBUG_D(fmt, ...)              do{                                              \
                                            printf(fmt, ##__VA_ARGS__);             \
                                            hlogd(fmt, ##__VA_ARGS__);              \
                                        }while(0);

#endif