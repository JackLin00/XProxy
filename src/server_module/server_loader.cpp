#include "hv/hloop.h"
#include "hv/hbase.h"
#include "hv/hsocket.h"
#include "hv/hv.h"
#include "server_loader.h"
#include "user_log.h"
#include "cmd_codec.h"
#include "command_handle_s.h"
#include <stdio.h>
#include <string>

unpack_setting_t unpack_setting;


static void on_close(hio_t* io) {
    DEBUG("on_close fd={} error={}", hio_fd(io), hio_error(io));
    XproxyOnClose(io);
}

static void on_recv(hio_t* io, void* buf, int readbytes) {
    DEBUG("on_recv fd={} readbytes={}", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    DEBUG("[{}] <=> [{}]",
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    CommandHandle((char*)buf, readbytes, io);
}


static void on_accept(hio_t* io) {
    DEBUG("on_accept connfd={}", hio_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    DEBUG("accept connfd={} [{}] <= [{}]", hio_fd(io),
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    hio_setcb_close(io, on_close);
    hio_setcb_read(io, on_recv);
    hio_set_unpack(io, &unpack_setting);
    hio_read_start(io);
}


void ServerLoader(ServerLoaderParam_t *param) {
#ifdef ENABLE_LUA
    param->lua_state = InitLuaState((void*)param->ini_parser);
    auto lua_file_path = param->ini_parser->GetValue("lua_file_path");
    INFO("lua file name {}", lua_file_path);
    if( !lua_file_path.empty() ){
        if( luaL_dofile(param->lua_state, lua_file_path.c_str()) != 0 ){
            ERROR("load lua file error : {}", lua_tostring(param->lua_state, -1));
            lua_pop(param->lua_state, 1);
        }
    }
#endif

    int max_proxy_port = param->ini_parser->Get<int>("allow_port_max_range");
    int min_proxy_port = param->ini_parser->Get<int>("allow_port_min_range");

    if( min_proxy_port >= max_proxy_port ){
        ERROR("allow_port_min_range bigger than allow_port_max_range. please check your config file.");
        exit(-1);
    }

    std::string host = param->ini_parser->GetValue("host");
    auto network_interface = param->ini_parser->GetValue("network_interface");
    int port = param->ini_parser->Get<int>("port");
    SET_LOG_LEVEL(spdlog::level::info);
    INFO("host : {}:{}, interface : {}", host.c_str(), port, network_interface);

    hloop_t* loop = hloop_new(0);
    hio_t* listenio = hloop_create_tcp_server(loop, "0.0.0.0", port, on_accept);
    if (listenio == NULL) {
        hloop_free(&loop);
        exit(-1);
    }

    if( !network_interface.empty() ){
        if( setsockopt(hio_fd(listenio), SOL_SOCKET, SO_BINDTODEVICE, network_interface.c_str(), network_interface.length()) == -1 ){
            ERROR("set interface error");
            exit(-1);
        }
    }

    memset((void*)&unpack_setting, 0, sizeof(unpack_setting_t));
    unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    unpack_setting.mode = UNPACK_BY_DELIMITER;
    unpack_setting.delimiter[0] = TAIL_CHAR;
    unpack_setting.delimiter_bytes = 1;

    hloop_set_userdata(loop, param);

    hloop_run(loop);
    hloop_free(&loop);
}