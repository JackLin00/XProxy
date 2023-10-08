extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "hv/iniparser.h"


static IniParser *use_handle{nullptr};

extern "C" void SetupIniHandle(void *handle){
    use_handle = (IniParser *)handle;
}

static int conf_get(lua_State *L){
    if(use_handle == nullptr){
        luaL_error(L, "init error.");
    }
    int top = lua_gettop(L);  /* depth of the stack */
    if( top != 2 ){
        luaL_error(L, "The number of parameters is incorrect. The required number is 2.");
    }

    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);

    std::string key = param1;
    std::string section = param2;

    auto rsp = use_handle->GetValue(key, section);

    lua_pushstring(L, rsp.c_str());
    return 1;
}

static int conf_set(lua_State *L){
    if(use_handle == nullptr){
        luaL_error(L, "init error.");
    }
    int top = lua_gettop(L);  /* depth of the stack */
    if( top != 3 ){
        luaL_error(L, "The number of parameters is incorrect. The required number is 3.");
    }

    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    const char *param3 = luaL_checkstring(L, 3);

    std::string key = param1;
    std::string value = param2;
    std::string section = param3;

    use_handle->SetValue(key, value, section);
    return 0;
}


static const luaL_Reg conf_funcs[] = {
    {"get", conf_get},
    {"set", conf_set},
    {NULL, NULL}
};


extern "C" int luaopen_conf(lua_State *L) {
    luaL_newlib(L, conf_funcs);
    return 1;
}