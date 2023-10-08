#ifndef __LOAD_LUA_S_H__
#define __LOAD_LUA_S_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

lua_State* InitLuaState();
int CallXProxycConnectHandle(lua_State* L, const char* json_string);

#ifdef __cplusplus
}
#endif



#endif