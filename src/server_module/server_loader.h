#ifndef __SERVER_LOADER_H__
#define __SERVER_LOADER_H__

#include "hv/iniparser.h"

#ifdef ENABLE_LUA
#include "lua_module_s.h"
#endif

typedef struct ServerLoaderParam{
    IniParser *ini_parser;
    #ifdef ENABLE_LUA
    lua_State* lua_state{NULL}; 
    #endif

    ~ServerLoaderParam(){
        #ifdef ENABLE_LUA
        if( lua_state != NULL ){
            lua_close(lua_state);
        }
        #endif
    }
}ServerLoaderParam_t;

void ServerLoader(ServerLoaderParam_t *);

#endif