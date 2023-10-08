#ifndef __COMMAND_PACKAGE_S_H__
#define __COMMAND_PACKAGE_S_H__


#include "cmd_codec.h"
#include <stdint.h>


#define    LOGIN_SUCCESS_CODE          0x00
#define    LOGIN_FAILD_CODE            0x01
#define    LOGIN_REJECT_BY_LUA_SCRIPT  0x02

CodecBuf_t PackageLoginRspCommand(uint8_t status);
CodecBuf_t PackageHeartBeatRspCommand();
CodecBuf_t PackageServiceClientOnConnectCommand(uint32_t client_id, uint32_t service_id);
CodecBuf_t PackageServiceClientOnDataCommand(uint32_t client_id, uint32_t service_id, const char *data, int data_len);
CodecBuf_t PackageServiceClientOnDisConnectCommand(uint32_t client_id, uint32_t service_id);




#endif