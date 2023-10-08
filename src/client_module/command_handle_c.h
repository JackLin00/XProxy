#ifndef __COMMAND_HANDLE_C_H__
#define __COMMAND_HANDLE_C_H__


#include "hv/iniparser.h"
#include "hv/hloop.h"

void SendLoginCommand(IniParser *, hio_t *);
void CommandHandle(char *buf, int len, hio_t *io);

#endif