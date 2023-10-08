#include "subservice_tcp.h"
#include "command_package_c.h"
#include "user_log.h"

extern hio_t *connect_xproxys_io;



static void on_message(hio_t* io, void* buf, int len) {
    SubServiceTcp *handle = (SubServiceTcp*)hevent_userdata(io);
    DEBUG("subservice tcp get data, client id : {}, service id: {}, len : {}", handle->self_client_id, handle->self_service_id, len);
#ifdef DEBUG_FLAG
    uint8_t *p = (uint8_t *)buf;
    for( int i = 0; i < len; i++){
        if( i % 16 == 0 )   printf("");
        printf("%02x ", p[i]);
    }
    printf("");
#endif

    CodecBuf_t send_data = PackageSubServiceOnData(handle->self_client_id, handle->self_service_id, (unsigned char*)buf, len);
#ifdef DEBUG_FLAG
    for( int i = 0; i < send_data.len; i++){
        if( i % 16 == 0 )   printf("");
        printf("%02x ", send_data.buf[i]);
    }
    printf("");
#endif

    hio_write(connect_xproxys_io, send_data.buf, send_data.len);
}

SubServiceTcp::SubServiceTcp(std::string url, int port, uint32_t client_id, uint32_t service_id, hio_t *io, DisconnectCallBackFunc_t func) {
    self_client_id = client_id;
    self_service_id = service_id;
    _func = func;
    self_io = hloop_create_tcp_client(hevent_loop(io), url.c_str(), port, 
    // 连接上的 callback
    [](hio_t *io){
        SubServiceTcp *handle = (SubServiceTcp*)hevent_userdata(io);
        CodecBuf_t send_data = PackageSubServiceOnConnect(handle->self_client_id, handle->self_service_id);
        hio_write(connect_xproxys_io, send_data.buf, send_data.len);

        // 开启读取数据
        hio_setcb_read(io, on_message);
        hio_read(io);
    }, 
    // 断开连接的 callback
    [](hio_t *io){
        SubServiceTcp *handle = (SubServiceTcp*)hevent_userdata(io);
        if( handle == NULL )        return;                 // 若已经被设置为 NULL，说明已经被清理
        CodecBuf_t send_data = PackageSubServiceOnDisConnect(handle->self_client_id, handle->self_service_id);
        hio_write(connect_xproxys_io, send_data.buf, send_data.len);
        hio_close(handle->self_io);
        handle->self_io = NULL;
        if( handle->_func != NULL ){
            handle->_func(handle->self_client_id, handle->self_service_id);
        }
    });
    hevent_set_userdata(self_io, this);
}

SubServiceTcp::~SubServiceTcp(){
    if( self_io != NULL ){
        hevent_set_userdata(self_io, NULL);
        hio_close(self_io);
    }
    INFO("subservice tcp delete, client id : {}, service id: {}", self_client_id, self_service_id);
}


void SubServiceTcp::GetRemoteData(const void *buf, int len){
    hio_write(self_io, buf, len);
}