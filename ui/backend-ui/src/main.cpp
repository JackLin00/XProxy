#include <iostream>
#include "hv/HttpServer.h"
#include <forward_list>

using namespace std;

#define   HTTP_SERVER_PORT                8889

struct DeviceItem{
    std::string ID;
    std::string name;
    std::string local_ip;
    std::string local_port;
    std::string remote_port;
};

std::forward_list<struct DeviceItem> device_tables;


int main(int argc, char** argv)
{
    HttpService router;
    http_server_t server;
    server.service = &router;
    server.port = HTTP_SERVER_PORT;

    router.GET("/get_devices", [](HttpRequest* req, HttpResponse* resp) {
        // cout << req->content_type << endl;
        // cout << req->body << endl;
        // cout << req->url << endl;


        // cout << "=============params=============" << endl;
        // for( auto &query_item : req->query_params ){
        //     cout << query_item.first << " = " << query_item.second << endl;
        // }

        // cout << "============headers==============" << endl;
        // for( auto &header_item : req->headers ){
        //     cout << header_item.first << " = " << header_item.second << endl;
        // }

        // cout << "=============json data=============" << endl;
        // if( req->content_type == APPLICATION_JSON ){
        //     auto xx_data = req->GetJson().dump();
        //     cout << "data : " << xx_data << endl;
        // }

        resp->json["devices"] = {};
        for( auto &device_item : device_tables ){
            hv::Json tmp;
            tmp["ID"] = device_item.ID;
            tmp["name"] = device_item.name;
            tmp["local_ip"] = device_item.local_ip;
            tmp["local_port"] = device_item.local_port;
            tmp["remote_port"] = device_item.remote_port;
            resp->json["devices"].push_back(tmp);
        }
        resp->SetHeader("Access-Control-Allow-Origin", "*");
        return 200;
    });

    router.POST("/device_change", [](HttpRequest* req, HttpResponse* resp) {
        if( req->content_type != APPLICATION_JSON ){
            return 400;
        }
        return resp->String("pong");
    });

    std::cout << "listen on: " << HTTP_SERVER_PORT << std::endl;

    http_server_run(&server);
    http_server_stop(&server);
    return 0; 
}
