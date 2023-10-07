#include "command_handle.h"
#include "cmd_codec.h"
#include "hv/json.hpp"
#include "hv/hbase.h"
#include "project_conf.h"
#include "user_log.h"
#include "command_package.h"
#include "subservice_tcp.h"
#include "hv/hdef.h"
#include "client_loader.h"
#include <vector>
#include <stdlib.h>
#include <string>
#include <unordered_map>


extern hio_t *connect_xproxys_io;

typedef struct ServiceClientItem{
    SubServiceBaseClass *sub_service_handle;
}ServiceClientItem_t;


typedef struct ServiceItem{
    int type;             // 0 为 TCP ，1 为 UDP
    std::string local_ip;
    int port;
    std::unordered_map<int, ServiceClientItem_t> client_table;
}ServiceItem_t;


static std::vector<ServiceItem_t> ServiceTable;

void SendLoginCommand(IniParser *parser, hio_t *io){
    nlohmann::json json_obj;
    auto service_list = parser->GetSections();
    if( service_list.empty() ){
        INFO("error, Can't get any service,exiting....");
        exit(-1);
    }

    json_obj["info"] = {};

    for( auto item : service_list ){
        nlohmann::json tmp_obj;
        auto type = parser->GetValue("type", item);
        auto url = parser->GetValue("local_ip", item);
        auto port = parser->Get<int>("local_port", item);

        ServiceItem service_item;

        // INFO("index : {}, service name : {}, local_ip : {}, local_port {}, type : {}", ServiceTable.size(), item.c_str(), url.c_str(), port, type.c_str());

        tmp_obj["index"] = ServiceTable.size();
        tmp_obj["name"] = item;
        tmp_obj["local_ip"] = url;
        tmp_obj["port"] = port;

        if( type == "TCP" || type == "tcp" ){
            service_item.type = 0;
        } else if( type == "UDP" || type == "udp" ){
            service_item.type = 1;
        } else {
            printf("Do not support %s type, please check it.", type.c_str());
            exit(-1);
        }
        json_obj["info"].push_back(tmp_obj);

        service_item.local_ip = url;
        service_item.port = port;
        ServiceTable.push_back(service_item);
    }

    std::string login_info = json_obj.dump(); 
    CodecBuf_t send_buf = PackageLoginReqCommand(login_info);
    hio_write(io, send_buf.buf, send_buf.len);
}

// 客户端登陆相关
static void HandleLoginRsp(const ProjectProtocol_t*, hio_t *io);
static void HandleHeartBeatRsp(const ProjectProtocol_t*, hio_t *io);

// 服务数据处理相关
static void HandleClientConnect(const ProjectProtocol_t*, hio_t *io);
static void HandleClientData(const ProjectProtocol_t*, hio_t *io);
static void HandleClientDisconnect(const ProjectProtocol_t*, hio_t *io);

const CommandItem_t CommandTable[] = {
    // 客户端登陆相关
    {LOGIN_CMD_RSP, HandleLoginRsp},

    // 服务数据处理相关
    {CONNECT_CMD_REQ, HandleClientConnect},
    {DISCONN_CMD_REQ, HandleClientDisconnect},
    {ON_DATA_CMD_REQ, HandleClientData},
    {HEARTBEAT_CMD_RSP, HandleHeartBeatRsp},
};



void CommandHandle(char *buf, int len, hio_t *io){
    CodecBuf_t recv_data = DecodeBuf(buf, len);
    // 需要先进行 checksum 计算
    uint16_t calc_sum = CalcCheckSum((const ProjectProtocol_t *)recv_data.buf, recv_data.len);
    // 然后进行字节序转换
    const ProjectProtocol_t *recv_payload = PreProcessCommand(recv_data.buf, recv_data.len);

    if( calc_sum != recv_payload->checksum ){
        INFO("check sum error. calc : {:x}, real : {:x}", calc_sum, recv_payload->checksum);
        return;
    }

#ifdef DEBUG_FLAG
    printf("get cmd : %x -- (%d:%d)", recv_payload->cmd, recv_payload->len, len);
#endif

    for(int i = 0; i < (int)ARRAY_SIZE(CommandTable); i++ ){
        if( recv_payload->cmd == CommandTable[i].cmd ){
            CommandTable[i].func_handle(recv_payload, io);
        }
    }
}

static void HandleLoginRsp(const ProjectProtocol_t* payload, hio_t *io){
    if( payload->param[0] == 0x00 ){
        INFO("login success");
    } else {
        INFO("login faild, code : {}", payload->param[0]);
    }
}

static void HandleHeartBeatRsp(const ProjectProtocol_t*, hio_t *io){
    ClientLoaderParam_t* client_param = (ClientLoaderParam_t*)hevent_userdata(io);
    client_param->heartbeat_handle->GetHeartBeatRsp();
}

static void HandleClientConnect(const ProjectProtocol_t* payload, hio_t *io){
    uint32_t client_id = GetU32FromBuffer((unsigned char*)payload->param);
    uint32_t service_id = GetU32FromBuffer((unsigned char*)&payload->param[4]);
    INFO("connect client id : {}, service id : {}", client_id, service_id);

    // 当服务的类型为 TCP 就需要创建 TCP client 去连接本地的服务器
    auto &service_handle = ServiceTable[service_id];
    if( service_handle.type == 0 ){
        // 创建 TCP 客户端 
        ServiceClientItem_t client_item;
        client_item.sub_service_handle = new SubServiceTcp(service_handle.local_ip, service_handle.port, client_id, service_id, io, [](uint32_t client_id, uint32_t service_id){
            // 当断开时，需要进行清理
            delete ServiceTable[service_id].client_table[client_id].sub_service_handle;
            ServiceTable[service_id].client_table.erase(client_id);
            INFO("subservice onclose : client id : {}, service id :{}", client_id, service_id);
        });
        service_handle.client_table[client_id] = client_item;
    }
}


// 客户端主动断开
static void HandleClientDisconnect(const ProjectProtocol_t* payload, hio_t *io){
    uint32_t client_id = GetU32FromBuffer((unsigned char*)payload->param);
    uint32_t service_id = GetU32FromBuffer((unsigned char*)&payload->param[4]);
    INFO("disconnect client id : {}, service id : {}", client_id, service_id);

    delete ServiceTable[service_id].client_table[client_id].sub_service_handle;
    ServiceTable[service_id].client_table.erase(client_id);
}

static void HandleClientData(const ProjectProtocol_t* payload, hio_t *io){
    uint32_t client_id = GetU32FromBuffer((unsigned char*)payload->param);
    uint32_t service_id = GetU32FromBuffer((unsigned char*)&payload->param[4]);
    DEBUG("on data client id : {}, service id : {}", client_id, service_id);

    if( ServiceTable[service_id].type == 0 ){
        // TCP 
        SubServiceTcp *tcp_handle = (SubServiceTcp *)ServiceTable[service_id].client_table[client_id].sub_service_handle;
        tcp_handle->GetRemoteData(&payload->param[8], payload->len - PROTOCOL_BASE_LEN - 8);
    }
}
