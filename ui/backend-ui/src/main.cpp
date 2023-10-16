#include <iostream>
#include "hv/HttpServer.h"
#include <forward_list>
#include <ctime>

using namespace std;

#define   HTTP_SERVER_PORT                8889

struct DeviceInfo{
    std::string name;
    std::string local_ip;
    int local_port;
    int remote_port;
};

struct DeviceItem{
    std::string ID;
    std::string login_date;
    std::forward_list<struct DeviceInfo> info;
};

std::forward_list<struct DeviceItem> device_tables;


int main(int argc, char** argv)
{
    HttpService router;
    http_server_t server;
    server.service = &router;
    server.port = HTTP_SERVER_PORT;

    router.GET("/get_devices", [](HttpRequest* req, HttpResponse* resp) {
        resp->json["devices"] = {};
        for( auto &device_item : device_tables ){
            hv::Json tmp;
            tmp["ID"] = device_item.ID;
            tmp["date"] = device_item.login_date;
            tmp["info"] = {};
            for( auto &info_item : device_item.info ){
                hv::Json info_tmp;
                info_tmp["name"] = info_item.name;
                info_tmp["local_ip"] = info_item.local_ip;
                info_tmp["local_port"] = info_item.local_port;
                info_tmp["remote_port"] = info_item.remote_port;
                tmp["info"].push_back(info_tmp);
            }
            resp->json["devices"].push_back(tmp);
        }
        resp->SetHeader("Access-Control-Allow-Origin", "*");
        return 200;
    });

    router.POST("/device_change", [](HttpRequest* req, HttpResponse* resp) {
        if( req->content_type != APPLICATION_JSON ){
            return 400;
        }
        auto json_data = req->GetJson();
        try{
            if( json_data["act"] == "add" ){
                std::time_t now = std::time(nullptr);
                // 使用本地时间
                std::tm localTime = *std::localtime(&now);
                // 格式化输出日期和时间
                char formattedTime[100];
                std::strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &localTime);
                // add device
                struct DeviceItem tmp;
                json_data.at("ID");
                tmp.ID = json_data["ID"];
                tmp.login_date = formattedTime;
                for( auto begin = json_data["info"].cbegin(); begin != json_data["info"].cend(); ++begin ){
                    struct DeviceInfo info;
                    info.name = (*begin)["name"];
                    info.local_ip = (*begin)["local_ip"];
                    info.local_port = (*begin)["local_port"];
                    info.remote_port = (*begin)["remote_port"];
                    tmp.info.push_front(info);
                }
                device_tables.push_front(tmp);
            } else {
                json_data.at("ID");

                cout << "del device : " << json_data["ID"] << endl;
                // del data
                device_tables.remove_if([&](const struct DeviceItem &item) {
                    return item.ID == json_data["ID"];
                });
            }
            return 200;
        } catch(...){
            // param error
            return 400;
        }
    });

    std::cout << "listen on: " << HTTP_SERVER_PORT << std::endl;

    http_server_run(&server);
    http_server_stop(&server);
    return 0; 
}
