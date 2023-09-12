#ifndef __SUBSERVICE_TYPE_H__
#define __SUBSERVICE_TYPE_H__

#include <stdint.h>
#include "hv/hloop.h"

typedef void (*DisconnectCallBackFunc_t)(uint32_t client_id, uint32_t service_id);

class SubServiceBaseClass{
    public:
        hio_t *self_io{NULL};
        uint32_t self_client_id{0};
        uint32_t self_service_id{0};
    
        SubServiceBaseClass(){};
        virtual ~SubServiceBaseClass(){};
};




#endif