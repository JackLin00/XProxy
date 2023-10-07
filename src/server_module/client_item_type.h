#ifndef __CLIENT_ITEM_TYPE_H__
#define __CLIENT_ITEM_TYPE_H__

#include <forward_list>
// #include <vector>
#include <string>
#include <stdint.h>
#include "hv/hloop.h"


typedef struct ClientServiceItem{
    uint32_t index;                      // 索引号
    std::string service_name;            // 服务名称
    int service_port;                    // 服务端口
    int server_port;                     // 服务器负责转发的 TCP服务器端口
    hio_t *io;                           // 对应的 网络 IO
}ClientServiceItem_t;



typedef struct ClientServiceInfo{
    std::forward_list<ClientServiceItem_t> service_list;
}ClientServiceInfo_t;


typedef struct ClientServerMap{
    hio_t *parent_io;                           // 对应的父 IO handle
    int service_index;                          // 对应的服务索引
    std::forward_list<hio_t*> client_list;      // 对应的子服务 io
}ClientServerMap_t;





#endif