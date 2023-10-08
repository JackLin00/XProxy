#include "command_package_s.h"
#include "protocol.h"
#include <memory>


CodecBuf_t PackageLoginRspCommand(uint8_t status){
    PACKAGE_DATA_START(LOGIN_CMD_RSP, 1);
    *p_buffer = status;
    PACKAGE_DATA_END();
}

CodecBuf_t PackageHeartBeatRspCommand(){
    PACKAGE_DATA_START(HEARTBEAT_CMD_RSP, 0);
    PACKAGE_DATA_END();
}

CodecBuf_t PackageServiceClientOnConnectCommand(uint32_t client_id, uint32_t service_id){
    PACKAGE_DATA_START(CONNECT_CMD_REQ, sizeof(uint32_t) + sizeof(uint32_t));
    // 设置 client id 和 service id
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    PaddingU32ToBuffer(p_buffer, service_id);
    PACKAGE_DATA_END();
}

CodecBuf_t PackageServiceClientOnDataCommand(uint32_t client_id, uint32_t service_id, const char *data, int data_len){
    PACKAGE_DATA_START(ON_DATA_CMD_REQ, sizeof(uint32_t) + sizeof(uint32_t) + data_len);
    // 设置 client id 和 service id
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    p_buffer += PaddingU32ToBuffer(p_buffer, service_id);

    for( int i = 0; i < data_len; i++ ){
        *p_buffer++ = data[i];
    }

    PACKAGE_DATA_END();
}

CodecBuf_t PackageServiceClientOnDisConnectCommand(uint32_t client_id, uint32_t service_id){
    PACKAGE_DATA_START(DISCONN_CMD_REQ, sizeof(ProjectProtocol_t) + sizeof(uint32_t) + sizeof(uint32_t));
    // 设置 client id 和 service id
    p_buffer += PaddingU32ToBuffer(p_buffer, client_id);
    PaddingU32ToBuffer(p_buffer, service_id);
    PACKAGE_DATA_END();
}