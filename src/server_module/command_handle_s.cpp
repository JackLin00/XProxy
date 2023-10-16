#include "command_handle_s.h"
#include "user_log.h"
#include "cmd_codec.h"
#include "hv/json.hpp"
#include "hv/hbase.h"
#include "hv/hsocket.h"
#include "client_item_type.h"
#include "command_package_s.h"
#include "project_conf.h"
#include "server_loader.h"
#ifdef ENABLE_LUA
#include "load_lua_s.h"
#endif
#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <arpa/inet.h>
#include <netinet/in.h>



#define    DEFAULT_SERVER_PROXY_MIN_PORT                1023
#define    DEFAULT_SERVER_PROXY_MAX_PORT                65536

static std::unordered_map<uint32_t, ClientServiceInfo_t> clients_table;
static std::unordered_map<int, ClientServerMap_t> clients_server_map;
static std::unordered_map<uint32_t, hio_t*> subservice_clients;


static void ProxyOnClose(hio_t *io){
    int local_port = ntohs(((struct sockaddr_in*)hio_localaddr(io))->sin_port);
    uint32_t client_id = hio_id(io);

    if(clients_server_map.count(local_port) > 0 && !hio_is_closed(clients_server_map[local_port].parent_io)){
        // parent io is not close
        CodecBuf_t send_buf = PackageServiceClientOnDisConnectCommand(client_id, clients_server_map[local_port].service_index);
        hio_write(clients_server_map[local_port].parent_io, send_buf.buf, send_buf.len);
    }

    DEBUG("disconnect, client id :{}, service id :{}, io : {}", client_id, clients_server_map[local_port].service_index, fmt::ptr(io));
}


static void ProxyOnData(hio_t *io, void* buf, int readbytes){
    int local_port = ntohs(((struct sockaddr_in*)hio_localaddr(io))->sin_port);
    uint32_t client_id = hio_id(io);
    CodecBuf_t send_buf = PackageServiceClientOnDataCommand(client_id, clients_server_map[local_port].service_index, (const char*)buf, readbytes);
    if(hio_is_closed(clients_server_map[local_port].parent_io)){
        // when parent io is close ?
        hio_close(io);                              // close self
        clients_server_map.erase(local_port);       // delete this data
    } else {
        hio_write(clients_server_map[local_port].parent_io, send_buf.buf, send_buf.len);
    }
}

static void ProxyOnAccet(hio_t* io) {
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    DEBUG("proxy accept connfd={} [{}] <= [{}]", hio_fd(io),
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    hio_setcb_close(io, ProxyOnClose);
    hio_setcb_read(io, ProxyOnData);
    hio_read_start(io);

    uint32_t client_id = hio_id(io);

    subservice_clients[client_id] = io;

    int local_port = ntohs(((struct sockaddr_in*)hio_localaddr(io))->sin_port);
    CodecBuf_t send_buf = PackageServiceClientOnConnectCommand(client_id, clients_server_map[local_port].service_index);
    hio_write(clients_server_map[local_port].parent_io, send_buf.buf, send_buf.len);
    clients_server_map[local_port].client_list.push_front(io);
}

static void HandleLogin(const ProjectProtocol_t*, hio_t *);
static void HandleHeartBeat(const ProjectProtocol_t*, hio_t *);
static void HandleSubServiceOnData(const ProjectProtocol_t*, hio_t *);
static void HandleSubServiceDisConnect(const ProjectProtocol_t*, hio_t *);

const CommandItem_t CommandTable[] = {
    {LOGIN_CMD_REQ, HandleLogin},
    {HEARTBEAT_CMD_REQ, HandleHeartBeat},
    {ON_DATA_CMD_RSP, HandleSubServiceOnData},
    {DISCONN_CMD_RSP, HandleSubServiceDisConnect}
};

void CommandHandle(char *buf, int len, hio_t *io){
    CodecBuf_t recv_data = DecodeBuf(buf, len);
    // 需要先进行 checksum 计算
    uint16_t calc_sum = CalcCheckSum((const ProjectProtocol_t *)recv_data.buf, recv_data.len);
    // 然后进行字节序转换
    // const ProjectProtocol_t *recv_payload = PreProcessCommand(recv_data.buf, recv_data.len);
    const ProjectProtocol_t *recv_payload = (const ProjectProtocol_t *)recv_data.buf;

    if( calc_sum != recv_payload->checksum ){
        ERROR("check sum error. calc : [{:x}], real : [{:x}]", calc_sum, recv_payload->checksum);
        return;
    }

#ifdef DEBUG_FLAG
    printf("get cmd : %x -- (%d:%d)", recv_payload->cmd, recv_payload->len, len);
    for( int i = 0; i < recv_data.len; i++ ){
        if( i % 16 == 0)   printf("");
        printf("%02x ", recv_data.buf[i]);
    }
    printf("");
#endif


    for(int i = 0; i < (int)ARRAY_SIZE(CommandTable); i++ ){
        if( recv_payload->cmd == CommandTable[i].cmd ){
            CommandTable[i].func_handle(recv_payload, io);
        }
    }
}

void XproxyOnClose(hio_t *io){
    uint32_t id = hio_id(io);
    auto &services = clients_table[id];
#ifdef ENABLE_LUA
    nlohmann::json data;
    data["ID"] = services.ID;
    ServerLoaderParam_t *loader_param = (ServerLoaderParam_t *)hloop_userdata(hevent_loop(io));
    CallXProxycLoginOutHandle(loader_param->lua_state, data.dump().c_str());
#endif
    for( auto index : services.service_list ){
        // 关闭已经连接该服务的所有子连接
        for( auto &client_item : clients_server_map[index.server_port].client_list ){
            hio_close(client_item);
        }

        // clear data
        clients_server_map.erase(index.server_port);
        // 关闭所有已经打开的服务
        hio_close(index.io);
    }

    // 去掉这个客户端的保存信息
    clients_table.erase(id);
}

static void XProxycWriteCallBack(hio_t* io, const void* buf, int writebytes){
    // when write success, just close it
    hio_close(io);
}

static void HandleLogin(const ProjectProtocol_t* payload, hio_t *io){
    nlohmann::json data;
    try{
        data = nlohmann::json::parse((const char*)payload->param);
    } catch(...){
        // invaild json data
        CodecBuf_t send_buf = PackageLoginRspCommand(LOGIN_FAILD_CODE);
        hio_setcb_write(io, XProxycWriteCallBack);
        hio_write(io, send_buf.buf, send_buf.len);
        return;
    }

#ifdef ENABLE_LUA
    // call lua script frist to check 
    ServerLoaderParam_t *param = (ServerLoaderParam_t *)hloop_userdata(hevent_loop(io));
    if( CallXProxycConnectHandle(param->lua_state, (const char*)payload->param) != 0 ){
        // lua script reject connect
        CodecBuf_t send_buf = PackageLoginRspCommand(LOGIN_REJECT_BY_LUA_SCRIPT);
        hio_setcb_write(io, XProxycWriteCallBack);
        hio_write(io, send_buf.buf, send_buf.len);
        return;
    }
#endif

    // 创建服务
    int service_len = data["info"].size();
    ClientServiceInfo_t service_info;
    service_info.ID = data["ID"];

    ServerLoaderParam_t *loader_param = (ServerLoaderParam_t *)hloop_userdata(hevent_loop(io));
    auto network_interface = loader_param->ini_parser->GetValue("network_interface");

    int max_proxy_port = loader_param->ini_parser->Get<int>("allow_port_max_range");
    int min_proxy_port = loader_param->ini_parser->Get<int>("allow_port_min_range");

    max_proxy_port = max_proxy_port == 0 ? DEFAULT_SERVER_PROXY_MIN_PORT : max_proxy_port;
    min_proxy_port = min_proxy_port == 0 ? DEFAULT_SERVER_PROXY_MIN_PORT : min_proxy_port;

    for( int i = 0; i < service_len; i++){
        ClientServiceItem_t service_item;
        ClientServerMap_t map_item;
        service_item.index = (uint32_t)data["info"][i]["index"];
        service_item.service_name = data["info"][i]["name"];
        service_item.local_ip = data["info"][i]["local_ip"];
        service_item.local_port = data["info"][i]["port"];
        int proxy_port = data["info"][i]["remote_port"];

        int rand_port;
        if( proxy_port != 0 && (proxy_port >= min_proxy_port) && (proxy_port <= max_proxy_port))     rand_port = proxy_port;
        else                                                                                         rand_port = hv_rand(min_proxy_port, max_proxy_port);

        // 每一个服务都需要创建一个 TCP 服务器
        hio_t *serivce_io = hloop_create_tcp_server(hevent_loop(io), "0.0.0.0", rand_port, ProxyOnAccet);
        while( serivce_io == NULL ){
            // 若存在端口占用的 case ，需要重试
            rand_port = hv_rand(min_proxy_port, max_proxy_port);
            serivce_io = hloop_create_tcp_server(hevent_loop(io), "0.0.0.0", rand_port, ProxyOnAccet);
        }

        if( !network_interface.empty() ){
            if( setsockopt(hio_fd(serivce_io), SOL_SOCKET, SO_BINDTODEVICE, network_interface.c_str(), network_interface.length()) == -1 ){
                ERROR("set interface error");
                exit(-1);
            }
        }

        map_item.parent_io = io;
        map_item.service_index = service_item.index;
        clients_server_map[rand_port] = map_item;

        service_item.server_port = rand_port;
        service_item.io = serivce_io;

        service_info.service_list.push_front(std::move(service_item));
    }

    clients_table[hio_id(io)] = std::move(service_info);
#ifdef ENABLE_LUA
    nlohmann::json device_info_table;
    device_info_table["ID"] = data["ID"];
    device_info_table["info"] = {};
#endif

    INFO("Get Device Connect (ID) : {}", data["ID"].dump());
    for( auto &item : clients_table ){
        for( auto list_item : item.second.service_list ){
#ifdef ENABLE_LUA
            nlohmann::json tmp;
            tmp["name"] = list_item.service_name;
            tmp["local_ip"] = list_item.local_ip;
            tmp["local_port"] = list_item.local_port;
            tmp["remote_port"] = list_item.server_port;
            device_info_table["info"].push_back(tmp);
#endif
            INFO("index : {}, service name : {}, service port : {}, server port : {}", list_item.index, list_item.service_name.c_str(), list_item.local_port, list_item.server_port);
        }
    }
#ifdef ENABLE_LUA
    std::string device_info_string = device_info_table.dump();
    CallAfterXProxycConnectHandle(param->lua_state, device_info_string.c_str());
#endif

    // 进行回复
    CodecBuf_t send_buf = PackageLoginRspCommand(LOGIN_SUCCESS_CODE);
    hio_write(io, send_buf.buf, send_buf.len);
}

static void HandleHeartBeat(const ProjectProtocol_t* payload, hio_t *io){
    CodecBuf_t send_buf = PackageHeartBeatRspCommand();
    hio_write(io, send_buf.buf, send_buf.len);
}


// XProxyc 那边主动断开子服务的连接
static void HandleSubServiceDisConnect(const ProjectProtocol_t* payload, hio_t *io){
    uint32_t client_id = GetU32FromBuffer((unsigned char*)payload->param);
    uint32_t service_id = GetU32FromBuffer((unsigned char*)&payload->param[4]);
    DEBUG("passivity disconnect client id : {}, service id : {}", client_id, service_id);
    hio_close(subservice_clients[client_id]);
}

// XProxyc 那边子服务存在数据
static void HandleSubServiceOnData(const ProjectProtocol_t* payload, hio_t* io){
    uint32_t client_id = GetU32FromBuffer((unsigned char*)payload->param);
    uint32_t service_id = GetU32FromBuffer((unsigned char*)&payload->param[4]);

    DEBUG("on data client id : {}, service id :{}, send len : {}", client_id, service_id, payload->len - PROTOCOL_BASE_LEN - 8);
    // 将数据写入到对应句柄中 
    hio_write(subservice_clients[client_id], (const void*)&payload->param[8], payload->len - PROTOCOL_BASE_LEN - 8);
}