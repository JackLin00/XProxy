#include "hv/hloop.h"
#include "hv/hbase.h"
#include "hv/hsocket.h"
#include "hv/hv.h"
#include "server_loader.h"
#include "user_log.h"
#include "cmd_codec.h"
#include "command_handle.h"
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
    std::string host = param->ini_parser->GetValue("host", "common");
    int port = param->ini_parser->Get<int>("port", "common");
    SET_LOG_LEVEL(spdlog::level::debug);
    DEBUG("host : {}:{}", host.c_str(), port);

    INFO("lua file name {}", param->ini_parser->GetValue("lua_file_path", "common"));

    hloop_t* loop = hloop_new(0);
    hio_t* listenio = hloop_create_tcp_server(loop, "0.0.0.0", port, on_accept);
    if (listenio == NULL) {
        hloop_free(&loop);
        exit(-1);
    }


#ifdef ENABLE_LUA
    param->lua_state = InitLuaState();
    auto lua_file_path = param->ini_parser->GetValue("lua_file_path", "common");
    if( !lua_file_path.empty() ){
        if( luaL_dofile(param->lua_state, lua_file_path.c_str()) != 0 ){
            ERROR("load lua file error : {}", lua_tostring(param->lua_state, -1));
            lua_pop(param->lua_state, 1);
        }
    }
#endif

    memset((void*)&unpack_setting, 0, sizeof(unpack_setting_t));
    unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    unpack_setting.mode = UNPACK_BY_DELIMITER;
    unpack_setting.delimiter[0] = TAIL_CHAR;
    unpack_setting.delimiter_bytes = 1;

    hloop_set_userdata(loop, param);

    hloop_run(loop);
    hloop_free(&loop);
}