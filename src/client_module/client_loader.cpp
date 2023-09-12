#include "client_loader.h"
#include "command_handle.h"
#include <stdio.h>
#include "hv/hv.h"
#include "hv/hloop.h"


hio_t *connect_xproxys_io = NULL;


static void on_message(hio_t* io, void* buf, int len) {
    CommandHandle((char*)buf, len, io);
}

static void on_connect(hio_t* io) {
    IniParser* parser = (IniParser*)hevent_userdata(io);
    printf("onconnect: connfd=%d\n", hio_fd(io));
    static unpack_setting_t s_unpack_setting;
    s_unpack_setting.mode = UNPACK_BY_DELIMITER;
    s_unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    s_unpack_setting.delimiter_bytes = 1;
    s_unpack_setting.delimiter[0] = 0x7e;
    hio_set_unpack(io, &s_unpack_setting);

    hio_setcb_read(io, on_message);
    hio_read(io);

    SendLoginCommand(parser, io);
}

static void on_close(hio_t* io) {
    printf("onclose: connfd=%d error=%d\n", hio_fd(io), hio_error(io));
}

void ClientLoader(IniParser *parser){
    hloop_t *loop = NULL;
    std::string host = parser->GetValue("server_host");
    int port = parser->Get<int>("server_port");

    printf("server url : %s:%d\n", host.c_str(), port);
    // 连接服务器
    loop = hloop_new(0);
    if( loop == NULL ){
        printf("error, can't create hloop\n");
        exit(-1);
    }

    connect_xproxys_io = hio_create_socket(loop, host.c_str(), port, HIO_TYPE_TCP, HIO_CLIENT_SIDE);
    if( connect_xproxys_io == NULL ){
        printf("error, can't connect socket\n");
        exit(-1);
    }
    hevent_set_userdata(connect_xproxys_io, parser);
    hio_setcb_connect(connect_xproxys_io, on_connect);
    hio_setcb_close(connect_xproxys_io, on_close);
    hio_connect(connect_xproxys_io);

    hloop_run(loop);
    hloop_free(&loop);
}