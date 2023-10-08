#ifndef __COMMAND_HANDLE_S_H__
#define __COMMAND_HANDLE_S_H__


#include "hv/hloop.h"


void CommandHandle(char *buf, int len, hio_t *io);
void XproxyOnClose(hio_t *io);

#endif