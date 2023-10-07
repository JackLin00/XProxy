#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include "hv/hloop.h"
#include <ctime>


class HeartBeat{
    private:
        htimer_t* _timer;
        hio_t* _service_io;
        time_t _record_time;

    public:
        HeartBeat(hloop_t *loop, hio_t *service_io);
        ~HeartBeat();

        void GetHeartBeatRsp();
        bool SendAndCheckTime();
        void DisconnectXProxy();
};



#endif