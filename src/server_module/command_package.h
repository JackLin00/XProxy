#ifndef __COMMAND_PACKAGE_H__
#define __COMMAND_PACKAGE_H__


#include "cmd_codec.h"
#include <stdint.h>

CodecBuf_t PackageLoginRspCommand(uint8_t status);
CodecBuf_t PackageServiceClientOnConnectCommand(uint32_t client_id, uint32_t service_id);
CodecBuf_t PackageServiceClientOnDataCommand(uint32_t client_id, uint32_t service_id, const char *data, int data_len);
CodecBuf_t PackageServiceClientOnDisConnectCommand(uint32_t client_id, uint32_t service_id);




#endif