#include "load_lua_s.h"

extern int UserPrint(lua_State* L);
extern int luaopen_cjson(lua_State* L);
extern void SetupIniHandle(void *handle);
extern int luaopen_conf(lua_State *L);

#ifdef ENABLE_LUA_REQUESTS
int luaopen_requests(lua_State *L);
#endif

lua_State* InitLuaState(void *conf_handle){
    lua_State* ret = luaL_newstate();
    // loading base lib
    luaL_openlibs(ret);
    luaopen_cjson(ret);

#ifdef ENABLE_LUA_REQUESTS
    luaL_requiref(ret, "requests", luaopen_requests, 1);
    lua_pop(ret, 1);
#endif

    SetupIniHandle(conf_handle);
    luaL_requiref(ret, "conf", luaopen_conf, 1);
    lua_pop(ret, 1);

    lua_setglobal(ret, "cjson");
    lua_pushcfunction(ret, UserPrint);
    lua_setglobal(ret, "print");
    return ret;
}

int CallXProxycConnectHandle(lua_State* L, const char* json_string){
    lua_getglobal(L, "OnXProxycLogin");
    if( !lua_isfunction(L, -1) ){
        lua_pop(L, 1);
        return 0;
    }

    lua_getglobal(L, "cjson");
    lua_getfield(L, -1, "decode");
    lua_remove(L, -2);

    lua_pushstring(L, json_string);
    if( lua_pcall(L, 1, 1, 0) != LUA_OK ){
        printf("error calling func cjson.decode, reson : %s\n", lua_tostring(L, -1));
        lua_pop(L, -1);
        return 0;
    }
    if( lua_pcall(L, 1, 1, 0) != LUA_OK ){
        printf("error calling func OnXProxycLogin\n");
        return 0;
    }

    int func_ret = lua_tointeger(L, -1);
    lua_pop(L, -1);
    return func_ret;
}