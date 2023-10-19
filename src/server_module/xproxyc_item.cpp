#include "xproxyc_item.h"
#include "server_loader.h"
#include "xproxys_conf.h"
#include "hv/hbase.h"
#include "user_log.h"

XProxyItem::XProxyItem(nlohmann::json& login_json_data, hloop_t *loop_handle){
    // get hloop param
    ServerLoaderParam_t *loader_param = (ServerLoaderParam_t *)hloop_userdata(loop_handle);
    // padding data
    _ID = login_json_data["ID"];
    _network_interface = loader_param->ini_parser->GetValue("network_interface");

    int max_proxy_port = loader_param->ini_parser->Get<int>("allow_port_max_range");
    int min_proxy_port = loader_param->ini_parser->Get<int>("allow_port_min_range");

    max_proxy_port = max_proxy_port == 0 ? DEFAULT_SERVER_PROXY_MIN_PORT : max_proxy_port;
    min_proxy_port = min_proxy_port == 0 ? DEFAULT_SERVER_PROXY_MIN_PORT : min_proxy_port;
    int service_len = login_json_data["info"].size();

    for( int i = 0; i < service_len; i++ ){
        ClientServiceItem_t service_item;
        service_item.index = (uint32_t)login_json_data["info"][i]["index"];
        service_item.service_name = login_json_data["info"][i]["name"];
        service_item.local_ip = login_json_data["info"][i]["local_ip"];
        service_item.local_port = login_json_data["info"][i]["port"];
        int proxy_port = login_json_data["info"][i]["remote_port"];

        int rand_port;
        if( proxy_port != 0 && (proxy_port >= min_proxy_port) && (proxy_port <= max_proxy_port))     rand_port = proxy_port;
        else                                                                                         rand_port = hv_rand(min_proxy_port, max_proxy_port);

        // 每一个服务都需要创建一个 TCP 服务器
        hio_t *serivce_io = hloop_create_tcp_server(loop_handle, "0.0.0.0", rand_port, ProxyOnAccet);
        while( serivce_io == NULL ){
            // 若存在端口占用的 case ，需要重试
            rand_port = hv_rand(min_proxy_port, max_proxy_port);
            serivce_io = hloop_create_tcp_server(loop_handle, "0.0.0.0", rand_port, ProxyOnAccet);
        }

        if( !_network_interface.empty() ){
            if( setsockopt(hio_fd(serivce_io), SOL_SOCKET, SO_BINDTODEVICE, _network_interface.c_str(), _network_interface.length()) == -1 ){
                ERROR("set interface error");
                exit(-1);
            }
        }
    }
}

XProxyItem::~XProxyItem(){

}