#include "protocol.h"

#define BITS_GET_U32_LE_IMPL(p)          ((uint32_t)(*((p) + 3) << 24 | *((p) + 2) << 16 | *((p) + 1) << 8 | *(p)))
#define BITS_GET_U32_BE_IMPL(p)          ((uint32_t)(*(p) << 24 | *((p) + 1) << 16 | *((p) + 2) << 8 | *((p) + 3)))
#define BITS_GET_U16_LE_IMPL(p)          ((uint16_t)((uint16_t)*((p) + 1) << 8 | (uint16_t)*(p)))
#define BITS_GET_U16_BE_IMPL(p)          ((uint16_t)(*((p)) << 8 | (uint16_t)*((p) + 1)))


// 字节序相关的函数
static bool IsLittleEndian(){
    int num = 1;
    return (*(char*)&num == 1);
}


static void BitsSetU32LeInline(unsigned char* p, uint32_t x)
{
    p[0] = (uint32_t)x;
    p[1] = (uint32_t)(x >> 8);
    p[2] = (uint32_t)(x >> 16);
    p[3] = (uint32_t)(x >> 24);
}

static void BitsSetU32BeInline(unsigned char* p, uint32_t x)
{
    p[0] = (uint32_t)(x >> 24);
    p[1] = (uint32_t)(x >> 16);
    p[2] = (uint32_t)(x >> 8);
    p[3] = (uint32_t)x;
}

static void BitsSetU16LeInline(unsigned char* p, uint16_t x)
{
    p[0] = (uint16_t)x;
    p[1] = (uint16_t)(x >> 8);
}

static void BitsSetU16BeInline(unsigned char* p, uint16_t x)
{
    p[0] = (uint16_t)(x >> 8);
    p[1] = (uint16_t)x;
}

int PaddingU32ToBuffer(unsigned char *p, uint32_t data){
    // 拷贝到 Buffer 统一转为 小端
    BitsSetU32LeInline(p, data);
    return 4;
}

int PaddingU16ToBuffer(unsigned char *p, uint16_t data){
    // 拷贝到 Buffer 统一转为 小端
    BitsSetU16LeInline(p, data);
    return 2;
}

uint32_t GetU32FromBuffer(unsigned char *p){
    return BITS_GET_U32_LE_IMPL(p);
}

uint16_t GetU16FromBuffer(unsigned char *p){
    return BITS_GET_U16_LE_IMPL(p);
}



ProjectProtocol_t* PreProcessCommand(unsigned char *buffer, int len){
    ProjectProtocol_t* ret = (ProjectProtocol_t*)buffer;
    if( !IsLittleEndian() ){
        // 当为大端时，需要进行处理
        ret->cmd = BITS_GET_U16_LE_IMPL(buffer);
        ret->len = BITS_GET_U16_LE_IMPL(buffer + 2);
        ret->checksum = BITS_GET_U16_LE_IMPL(buffer + 2 + 2);
    }
    return ret;
}

void CAutoFreeFunc(void *p){
    free(p);
}

void PaddingCmdAndLen(unsigned char *buffer, uint16_t cmd, uint16_t len){
    buffer += PaddingU16ToBuffer(buffer, cmd);          // cmd
    buffer += PaddingU16ToBuffer(buffer, len);          // len 
}