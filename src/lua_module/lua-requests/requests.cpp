extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "hv/requests.h"
#include "user_log.h"


static void PackageRequestRsp(lua_State *L, requests::Response &rsp, const char *url){
    lua_newtable(L);
    // url
    lua_pushstring(L, "url");
    lua_pushstring(L, url);
    lua_settable(L, -3);

    // status_code
    lua_pushstring(L, "status_code");
    lua_pushinteger(L, rsp->status_code);
    lua_settable(L, -3);

    // text
    lua_pushstring(L, "text");
    lua_pushstring(L, (const char*)rsp->Content());
    lua_settable(L, -3);

    // header
    lua_pushstring(L, "header");
    lua_newtable(L);
    for( auto& header_item : rsp->headers ){
        lua_pushstring(L, header_item.first.c_str());
        if( header_item.first == "Content-Length" ){
            lua_pushinteger(L, std::stoi(header_item.second));
        } else {
            lua_pushstring(L, header_item.second.c_str());
        }
        lua_settable(L, -3);
    }
    lua_settable(L, -3);
}

static int requests_get(lua_State *L){
    int top = lua_gettop(L);  /* depth of the stack */
    if( top == 0 ){
        luaL_error(L, "please input url as least.");
    }
    const char *url = luaL_checkstring(L, 1);

    auto rsp = requests::get(url);
    if( rsp == NULL ){
        lua_pushnil(L);
        return 1;
    }

    PackageRequestRsp(L, rsp, url);
    return 1;
}


static int requests_post(lua_State *L){
    int top = lua_gettop(L);  /* depth of the stack */
    if( top == 0 ){
        luaL_error(L, "please input url as least.");
    }
    const char *url = luaL_checkstring(L, 1);
    auto rsp = requests::post(url);
    if( rsp == NULL ){
        lua_pushnil(L);
        return 1;
    }

    PackageRequestRsp(L, rsp, url);
    return 1;
}


static const luaL_Reg request_funcs[] = {
    {"get", requests_get},
    {"post", requests_post},
    {NULL, NULL}
};

extern "C" int luaopen_requests(lua_State *L) {
    luaL_newlib(L, request_funcs);
    return 1;
}