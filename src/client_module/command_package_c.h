#ifndef __COMMAND_PACKAGE_C_H__
#define __COMMAND_PACKAGE_C_H__

#include "cmd_codec.h"
#include <stdint.h>
#include <string>



CodecBuf_t PackageLoginReqCommand(std::string& login_string);
CodecBuf_t PackageHeartBeatReqCommand();



CodecBuf_t PackageSubServiceOnConnect(uint32_t client_id, uint32_t service_id);
CodecBuf_t PackageSubServiceOnDisConnect(uint32_t client_id, uint32_t service_id);
CodecBuf_t PackageSubServiceOnData(uint32_t client_id, uint32_t service_id, unsigned char *data, int data_len);




#endif