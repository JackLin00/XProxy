#ifndef __CLIENT_LOADER_H__
#define __CLIENT_LOADER_H__

#include "hv/iniparser.h"
#include "hv/hloop.h"

typedef struct ClientLoaderParam{
    IniParser *ini_parser;
    hloop_t *loop;
    #ifdef ENABLE_LUA
    lua_State* lua_state{NULL}; 
    #endif

    ~ClientLoaderParam(){
        #ifdef ENABLE_LUA
        if( lua_state != NULL ){
            lua_close(lua_state);
        }
        #endif
    }
}ClientLoaderParam_t;


void ClientLoader(ClientLoaderParam_t *);

#endif