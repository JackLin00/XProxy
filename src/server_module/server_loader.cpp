#include "hv/hloop.h"
#include "hv/hbase.h"
#include "hv/hsocket.h"
#include "hv/hv.h"
#include "server_loader.h"
#include "cmd_codec.h"
#include "command_handle.h"
#include <stdio.h>
#include <string>

unpack_setting_t unpack_setting;


static void on_close(hio_t* io) {
    printf("on_close fd=%d error=%d\n", hio_fd(io), hio_error(io));
    XproxyOnClose(io);
}

static void on_recv(hio_t* io, void* buf, int readbytes) {
    printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    CommandHandle((char*)buf, readbytes, io);
}


static void on_accept(hio_t* io) {
    printf("on_accept connfd=%d\n", hio_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("accept connfd=%d [%s] <= [%s]\n", hio_fd(io),
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    hio_setcb_close(io, on_close);
    hio_setcb_read(io, on_recv);
    hio_set_unpack(io, &unpack_setting);
    hio_read_start(io);
}


void ServerLoader(IniParser *parser) {
    std::string host = parser->GetValue("host", "common");
    int port = parser->Get<int>("port", "common");
    printf("host : %s:%d\n", host.c_str(), port);

    hloop_t* loop = hloop_new(0);
    hio_t* listenio = hloop_create_tcp_server(loop, "0.0.0.0", port, on_accept);
    if (listenio == NULL) {
        hloop_free(&loop);
        exit(-1);
    }

    memset((void*)&unpack_setting, 0, sizeof(unpack_setting_t));
    unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    unpack_setting.mode = UNPACK_BY_DELIMITER;
    unpack_setting.delimiter[0] = TAIL_CHAR;
    unpack_setting.delimiter_bytes = 1;

    hloop_run(loop);
    hloop_free(&loop);
}