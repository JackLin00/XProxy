#ifndef __CMD_CODEC_H__
#define __CMD_CODEC_H__

#include <stdint.h>
#include "protocol.h"


typedef struct CodecBuf{
        int len;
        unsigned char *buf;
        ~CodecBuf(){
            if( buf && len > 0){
                delete[] buf;
            }
        }
}CodecBuf_t;

CodecBuf_t EncodeBuf(const char *buf, int len);
CodecBuf_t DecodeBuf(const char *buf, int len);
uint16_t CalcCheckSum(const ProjectProtocol_t *, uint16_t);


#endif