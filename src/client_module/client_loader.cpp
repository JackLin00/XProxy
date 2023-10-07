#include "client_loader.h"
#include "command_handle.h"
#include <stdio.h>
#include "hv/hv.h"
#include "hv/hloop.h"
#include "hv/htime.h"
#include "user_log.h"
#include "heartbeat.h"

#define    DEFAULT_RECONNECT_TIME                            20

hio_t *connect_xproxys_io = NULL;
static HeartBeat* heartbeat_handle{nullptr};


static void on_message(hio_t* io, void* buf, int len) {
    CommandHandle((char*)buf, len, io);
}

static void on_connect(hio_t* io) {
    ClientLoaderParam_t* client_param = (ClientLoaderParam_t*)hevent_userdata(io);
    static unpack_setting_t s_unpack_setting;
    s_unpack_setting.mode = UNPACK_BY_DELIMITER;
    s_unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    s_unpack_setting.delimiter_bytes = 1;
    s_unpack_setting.delimiter[0] = 0x7e;
    hio_set_unpack(io, &s_unpack_setting);

    hio_setcb_read(io, on_message);
    hio_read(io);

    SendLoginCommand(client_param->ini_parser, io);

    heartbeat_handle = new HeartBeat(client_param->loop, io);
    client_param->heartbeat_handle = heartbeat_handle;
}

static void on_close(hio_t* io);

static void ReconnectTimerCallback(htimer_t* timer){
    ClientLoaderParam_t* client_param = (ClientLoaderParam_t*)hevent_userdata(timer);
    std::string host = client_param->ini_parser->GetValue("server_host");
    int port = client_param->ini_parser->Get<int>("server_port");

    auto connect_xproxys_io = hio_create_socket(client_param->loop, host.c_str(), port, HIO_TYPE_TCP, HIO_CLIENT_SIDE);
    if( connect_xproxys_io == NULL ){
        ERROR("error, can't connect socket");
        exit(-1);
    }

    hevent_set_userdata(connect_xproxys_io, client_param);
    hio_setcb_connect(connect_xproxys_io, on_connect);
    hio_setcb_close(connect_xproxys_io, on_close);
    hio_connect(connect_xproxys_io);
    DEBUG("reconnecting XProxys");
}

static void on_close(hio_t* io) {
    ClientLoaderParam_t* client_param = (ClientLoaderParam_t*)hevent_userdata(io);
    int interval = client_param->ini_parser->Get<int>("reconnect_interval", "", DEFAULT_RECONNECT_TIME);
    INFO("disconnect from XProxyc, reconnect interval : {}s", interval);
    htimer_t* timer = htimer_add(client_param->loop, ReconnectTimerCallback, interval * 1000, 1);
    if( timer == NULL ){
        ERROR("Can't create timer.exiting....");
        exit(-1);
    }

    hevent_set_userdata(timer, client_param);
    delete heartbeat_handle;
    heartbeat_handle = nullptr;
}

void ClientLoader(ClientLoaderParam_t *param){
    hloop_t *loop = NULL;
    std::string host = param->ini_parser->GetValue("server_host");
    int port = param->ini_parser->Get<int>("server_port");

    INFO("server url : {}:{}", host, port);
    // 连接服务器
    loop = hloop_new(0);
    if( loop == NULL ){
        ERROR("error, can't create hloop");
        exit(-1);
    }

    param->loop = loop;

    connect_xproxys_io = hio_create_socket(loop, host.c_str(), port, HIO_TYPE_TCP, HIO_CLIENT_SIDE);
    if( connect_xproxys_io == NULL ){
        ERROR("error, can't connect socket");
        exit(-1);
    }
    hevent_set_userdata(connect_xproxys_io, param);
    hio_setcb_connect(connect_xproxys_io, on_connect);
    hio_setcb_close(connect_xproxys_io, on_close);
    hio_connect(connect_xproxys_io);

    hloop_run(loop);
    hloop_free(&loop);
}