extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "hv/requests.h"
#include "hv/hurl.h"
#include "user_log.h"


// static void stackDump (lua_State *L) {
//     printf("===================in print stack==========s==========\n");
//     int i;
//     int top = lua_gettop(L);  /* depth of the stack */
//     for (i = 1; i <= top; i++) {  /* repeat for each level */
//         int t = lua_type(L, i);
//         switch (t) {
//             case LUA_TSTRING: {  /* strings */
//                 printf("'%s'", lua_tostring(L, i));
//                 break;
//             }
//             case LUA_TBOOLEAN: {  /* Booleans */
//                 printf(lua_toboolean(L, i) ? "true" : "false");
//                 break;
//             }
//             case LUA_TNUMBER: {  /* numbers */
//                 printf("%g", lua_tonumber(L, i));
//                 break;
//             }
//             default: {  /* other values */
//                 printf("%s", lua_typename(L, t));
//                 break;
//             }
//         }
//         printf("  ");  /* put a separator */
//     }
//     printf("\n");  /* end the listing */
//     printf("===================in print stack===========e=========\n");
// }


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

static http_headers ParseRequestHeadersData(lua_State *L){
    http_headers ret;
    lua_pushstring(L, "headers");
    lua_gettable(L, -2);

    if( lua_type(L, -1) == LUA_TNIL ){
        lua_pop(L, 1);
        return ret;      // not exist
    }
    if( lua_type(L, -1) != LUA_TTABLE ){
        lua_pop(L, 1);
        luaL_error(L, "headers must be table.");
        return ret;
    }

    lua_pushnil(L);
    while( lua_next(L, -2) ){
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if( lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING ){
            ret[lua_tostring(L, -2)] = lua_tostring(L, -1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);       // pop json table
    return ret;
}

static http_body ParseRequestBodyData(lua_State *L){
    return "";
}

static std::string ParseRequestJsonData(lua_State *L){
    std::string ret;
    lua_pushstring(L, "json");
    lua_gettable(L, -2);

    if( lua_type(L, -1) == LUA_TNIL ) {
        lua_pop(L, 1);
        return ret;      // not exist
    }
    if( lua_type(L, -1) != LUA_TTABLE ){
        lua_pop(L, 1);
        luaL_error(L, "json must be table.");
        return ret;
    }

    // call cjson encode 
    lua_getglobal(L, "cjson");
    lua_getfield(L, -1, "encode");
    lua_pushvalue(L, -3);
    lua_pcall(L, 1, 1, 0);
    ret = (const char*)lua_tostring(L, -1);

    lua_pop(L, 1);       // pop ret
    lua_pop(L, 1);       // pop cjson table
    lua_pop(L, 1);       // pop json table
    return ret;
}

static std::string ParseRequtestParamData(lua_State *L, HUrl &url){
    std::string ret;
    lua_pushstring(L, "params");
    lua_gettable(L, -2);

    if( lua_type(L, -1) == LUA_TNIL ) {
        lua_pop(L, 1);
        return ret;      // not exist
    }
    if( lua_type(L, -1) != LUA_TTABLE ){
        lua_pop(L, 1);
        luaL_error(L, "params must be table.");
    }
    ret = url.query;
    lua_pushnil(L);
    while( lua_next(L, -2) ){
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if( lua_type(L, -2) == LUA_TSTRING && (lua_type(L, -1) == LUA_TSTRING || lua_type(L, -1) == LUA_TNUMBER ) ){
            std::string param_data = lua_tostring(L, -2);
            param_data += "=";
            if( lua_isinteger(L, -1) ){
                // int
                param_data += std::to_string(lua_tointeger(L, -1));
            } else if( lua_isnumber(L, -1) ){
                // number
                param_data += std::to_string(lua_tonumber(L, -1));
            } else {
                // string 
                param_data += lua_tostring(L, -1);
            }
            if( ret.empty() ){
                ret = param_data;
            } else {
                ret += "&" + param_data;
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);       // pop json table
    return ret;
}

static int requests_get(lua_State *L){
    int top = lua_gettop(L);  /* depth of the stack */
    if( top == 0 ){
        luaL_error(L, "please input url as least.");
    }
    std::string url_get = luaL_checkstring(L, 1);
    HUrl url_parse;
    if(!url_parse.parse(url_get)) {
        luaL_error(L, "url is invaild.");
    }

    http_headers headers;
    std::string param_data;
    if( top >= 2 ){
        luaL_checktype(L, 2, LUA_TTABLE);
        headers = ParseRequestHeadersData(L);
        param_data = ParseRequtestParamData(L, url_parse);
    }

    if( !param_data.empty() ){
        if( url_parse.query.empty() )          url_parse.url += "?" + param_data;
        else {
            auto pos = url_parse.url.find(url_parse.query);
            url_parse.url.replace(pos, url_parse.query.length(), param_data);
        }
    }

    auto &struct_url = url_parse.url;
    requests::Response rsp;
    rsp = requests::get(struct_url.c_str(), headers);
    if( rsp == NULL ){
        lua_pushnil(L);
        return 1;
    }

    PackageRequestRsp(L, rsp, struct_url.c_str());
    return 1;
}


static int requests_post(lua_State *L){
    int top = lua_gettop(L);  /* depth of the stack */
    if( top == 0 ){
        luaL_error(L, "please input url as least.");
    }
    std::string url_get = luaL_checkstring(L, 1);
    HUrl url_parse;
    if(!url_parse.parse(url_get)) {
        luaL_error(L, "url is invaild.");
    }

    http_headers headers;
    http_body body;
    std::string json_data;
    std::string param_data;

    if( top >= 2 ){
        luaL_checktype(L, 2, LUA_TTABLE);
        headers = ParseRequestHeadersData(L);
        body = ParseRequestBodyData(L);
        json_data = ParseRequestJsonData(L);
        param_data = ParseRequtestParamData(L, url_parse);
    }

    if( !json_data.empty() ){
        headers["Content-Type"] = "application/json";
        body = std::move(json_data);
    }

    if( !param_data.empty() ){
        if( url_parse.query.empty() )          url_parse.url += "?" + param_data;
        else {
            auto pos = url_parse.url.find(url_parse.query);
            url_parse.url.replace(pos, url_parse.query.length(), param_data);
        }
    }

    auto &struct_url = url_parse.url;
    requests::Response rsp;
    rsp = requests::post(struct_url.c_str(), body, headers);

    if( rsp == NULL ){
        lua_pushnil(L);
        return 1;
    }

    PackageRequestRsp(L, rsp, struct_url.c_str());
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