#include "command_package_c.h"
#include "protocol.h"
#include <memory>


CodecBuf_t PackageLoginReqCommand(std::string& login_string){
    PACKAGE_DATA_START(LOGIN_CMD_REQ, login_string.length() + 1);

    for( long unsigned int i = 0; i < login_string.length(); i++ ){
        *p_buffer++ = login_string.c_str()[i];
    }
    *p_buffer = 0x00;

    PACKAGE_DATA_END();
}

CodecBuf_t PackageHeartBeatReqCommand(){
    PACKAGE_DATA_START(HEARTBEAT_CMD_REQ, 0);
    PACKAGE_DATA_END();
}

CodecBuf_t PackageSubServiceOnConnect(uint32_t client_id, uint32_t service_id){
    PACKAGE_DATA_START(CONNECT_CMD_RSP, sizeof(uint32_t) + sizeof(uint32_t));
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    PaddingU32ToBuffer(p_buffer, service_id);
    PACKAGE_DATA_END();
}

CodecBuf_t PackageSubServiceOnDisConnect(uint32_t client_id, uint32_t service_id){
    PACKAGE_DATA_START(DISCONN_CMD_RSP, sizeof(uint32_t) + sizeof(uint32_t));
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    PaddingU32ToBuffer(p_buffer, service_id);
    PACKAGE_DATA_END();
}


CodecBuf_t PackageSubServiceOnData(uint32_t client_id, uint32_t service_id, unsigned char *data, int data_len){
    PACKAGE_DATA_START(ON_DATA_CMD_RSP, sizeof(uint32_t) + sizeof(uint32_t) + data_len);
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    p_buffer += PaddingU32ToBuffer(p_buffer, service_id);

    for( int i = 0; i < data_len; i++ ){
        *p_buffer++ = data[i];
    }
    PACKAGE_DATA_END();
}