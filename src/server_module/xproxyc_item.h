#ifndef __XPROXYC_ITEM_H__
#define __XPROXYC_ITEM_H__

#include "hv/json.hpp"
#include "hv/hloop.h"
#include "client_item_type.h"
#include <forward_list>

class XProxyItem{
    public:
        XProxyItem(nlohmann::json& login_json_data, hloop_t *loop_handle);
        ~XProxyItem();

        const std::string& GetID();
        const std::string& GetNetWorkInterfcae();
        const std::forward_list<ClientServiceItem_t>& GetServiceList;

    private:
        std::string _network_interface;
        std::string _ID;
        std::forward_list<ClientServiceItem_t> _service_list;
};


#endif