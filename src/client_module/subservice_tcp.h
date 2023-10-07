#ifndef __SUBSERVICE_TCP_H__
#define __SUBSERVICE_TCP_H__


#include "subservice_type.h"
#include <string>



class SubServiceTcp : public SubServiceBaseClass{
    public:
        DisconnectCallBackFunc_t _func{NULL};

        SubServiceTcp(std::string url, int port, uint32_t client_id, uint32_t service_id, hio_t *io, DisconnectCallBackFunc_t func);
        SubServiceTcp(const SubServiceTcp &) = delete;
        SubServiceTcp(const SubServiceTcp &&) = delete;
        ~SubServiceTcp() override;

        void GetRemoteData(const void *buf, int len);
};




#endif