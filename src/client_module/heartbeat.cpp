#include "heartbeat.h"
#include <ctime>
#include <stdexcept>
#include "command_package.h"
#include "user_log.h"

#define   HEARTBEAT_TIMEOUT                    10000
#define   HEARTBEAT_MAX_SECOND                 15

static void HeartBeat_Timeout(htimer_t* timer){
    HeartBeat* param = (HeartBeat*)hevent_userdata(timer);
    if( !param->SendAndCheckTime() ){
        param->DisconnectXProxy();
    }
}

HeartBeat::HeartBeat(hloop_t *loop, hio_t *service_io) : _service_io(service_io) {
    _timer = htimer_add(loop, HeartBeat_Timeout, HEARTBEAT_TIMEOUT, INFINITE);

    if( _timer == NULL ){
        throw std::runtime_error("can't create timer");
    }

    hevent_set_userdata(_timer, this);

    time(&_record_time);
}

HeartBeat::~HeartBeat(){
    htimer_del(_timer);
}

void HeartBeat::GetHeartBeatRsp(){
    time(&_record_time);
}

bool HeartBeat::SendAndCheckTime(){
    time_t now_time;
    CodecBuf_t send_buf = PackageHeartBeatReqCommand();
    hio_write(_service_io, send_buf.buf, send_buf.len);
    time(&now_time);
    DEBUG("now time : {}, record time : {}", now_time, _record_time);

    if( now_time - _record_time > HEARTBEAT_MAX_SECOND ){
        return false;
    }
    return true;
}

void HeartBeat::DisconnectXProxy(){
    if( _service_io != NULL ){
        hio_close(_service_io);
        _service_io = NULL;
    }
}
