#include "cmd_codec.h"


#define  ADD_PADDING_LEN         200
#define  FILTER_CHAR             0x7D
#define  XOR_CHAR                0x20

CodecBuf_t EncodeBuf(const char *buf, int len){
    CodecBuf_t ret;
    ret.buf = new unsigned char[len + 200];
    ret.len = 0;
    for(int i = 0; i < len; i++){
        if( buf[i] != TAIL_CHAR && buf[i] != FILTER_CHAR ){
            ret.buf[ret.len++] = buf[i];
            continue;
        }

        // 感兴趣的字符
        ret.buf[ret.len++] = 0x7D;
        ret.buf[ret.len++] = buf[i] ^ XOR_CHAR;
    }
    ret.buf[ret.len++] = TAIL_CHAR;
    return ret;
}

CodecBuf_t DecodeBuf(const char *buf, int len){
    CodecBuf_t ret;
    ret.buf = new unsigned char[len];
    ret.len = 0;

    for( int i = 0; i < len - 1; i++ ){
        if( buf[i] != FILTER_CHAR ){
            ret.buf[ret.len++] = buf[i];
            continue;
        }
        ret.buf[ret.len++] = buf[++i] ^ XOR_CHAR;
    }
    return ret;
}


uint16_t CalcCheckSum(const ProjectProtocol_t *payload, uint16_t len){
    const uint8_t *p = (const uint8_t *)payload;
    uint16_t ret = 0;

    for( uint16_t i = 0; i < len; i++){
        if( i == 4 || i == 5 || i == (len - 1) ){
            // checksum and tail ignore
            continue;
        }

        ret += p[i];
    }
    return ret;
}