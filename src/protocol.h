#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


#include <stdint.h>
#include "hv/hloop.h"

#define  TAIL_CHAR               0x7E
#define  DEFAULT_TAIL_SIZE       1

#define  PROTOCOL_BASE_LEN       6


// command table 
#define         LOGIN_CMD_REQ                   0x0000              // xproxyc 登陆命令
#define         LOGIN_CMD_RSP                   0x8000
#define         LOGOUT_CMD_REQ                  0x0001              // xproxyc 登出命令
#define         LOGOUT_CMD_RSP                  0x8001         
#define         CONNECT_CMD_REQ                 0x0010              // xproxys 有客户端连接上对应 socket
#define         CONNECT_CMD_RSP                 0x8010              // xproxyc 成功连接上对应的服务器
#define         ON_DATA_CMD_REQ                 0x0011              // xproxys 客户端有数据
#define         ON_DATA_CMD_RSP                 0x8011              // xproxyc 服务有数据返回
#define         DISCONN_CMD_REQ                 0x0012              // xproxys 客户端登出
#define         DISCONN_CMD_RSP                 0x8012              // xproxyc 客户端被动地断开连接

typedef struct ProjectProtocol{
    uint16_t cmd;
    uint16_t len;
    uint16_t checksum;
    uint8_t param[0];
}ProjectProtocol_t;


typedef struct CommandItem{
    uint16_t cmd;
    void (*func_handle)(const ProjectProtocol_t*, hio_t *io);
}CommandItem_t;

int PaddingU32ToBuffer(unsigned char *p, uint32_t data);
int PaddingU16ToBuffer(unsigned char *p, uint16_t data);
uint32_t GetU32FromBuffer(unsigned char *p);
uint16_t GetU16FromBuffer(unsigned char *p);
ProjectProtocol_t* PreProcessCommand(unsigned char *buffer, int len);
void CAutoFreeFunc(void *p);
void PaddingCmdAndLen(unsigned char *buffer, uint16_t cmd, uint16_t len);


#define        PACKAGE_DATA_START(cmd, len)           uint16_t malloc_len = sizeof(ProjectProtocol_t) + (len);           \
                                                      unsigned char *p_buffer = (unsigned char*)malloc(malloc_len);      \
                                                      unsigned char *fix_p_buffer = p_buffer;                            \
                                                      std::unique_ptr<unsigned char, decltype(&CAutoFreeFunc)> smart_ptr(fix_p_buffer, CAutoFreeFunc);    \
                                                      PaddingCmdAndLen(p_buffer, (cmd), (malloc_len));                          \
                                                      p_buffer += 6;

#define        PACKAGE_DATA_END()                     PaddingU16ToBuffer(fix_p_buffer + 4, CalcCheckSum((const ProjectProtocol_t*)fix_p_buffer, malloc_len));  \
                                                      return EncodeBuf((const char*)fix_p_buffer, malloc_len);





#endif