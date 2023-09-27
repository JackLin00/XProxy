#include "lua_module_s.h"

#define      DEPTH_PRINTF(depth, ...)                           do{                                                      \
                                                                    for(int i = 0; i < depth - 1; i++)      printf("  "); \
                                                                    printf(__VA_ARGS__);                                 \
                                                                }while(0)


extern int luaopen_cjson(lua_State *l);

// static void stackDump (lua_State *L) {
//     printf("printf stack:");
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
//             }
//             printf("  ");  /* put a separator */    
//     }
//     printf("\n");  /* end the listing */
// }

static void __PrintTableType(lua_State* L, int idx, int depth);
static void __PrintNormalType(lua_State* L, int idx, int depth){
    switch(lua_type(L, idx)){
        case LUA_TSTRING:
            DEPTH_PRINTF(depth, "\"%s\"", lua_tostring(L, idx));
            break;
        case LUA_TBOOLEAN:
            DEPTH_PRINTF(depth, lua_toboolean(L, idx) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            DEPTH_PRINTF(depth, "%g", lua_tonumber(L, idx));
            break;
        case LUA_TNIL:
            DEPTH_PRINTF(depth, "nil");
            break;
        case LUA_TTABLE:
            __PrintTableType(L, -1 - 1, depth + 1);
            break;
        default:
            DEPTH_PRINTF(depth, "Unknown");
    }
    // DEPTH_PRINTF(depth, "\n");
}


static void __PrintTableType(lua_State* L, int idx, int depth){
    int index = 0;
    lua_pushnil(L);
    DEPTH_PRINTF(depth, "{\n");
    while( lua_next(L, idx) ){
        if( index != 0 )        printf(",\n");
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if( lua_isinteger(L, -2) ){
            // int 作为索引
            __PrintNormalType(L, -1, depth + 1);
        } else {
            // 用字符作为键
            DEPTH_PRINTF(depth, "%s = ", lua_tostring(L, -2));
            if( lua_istable(L, -1) ){
                // 若为 table 则递归
                __PrintTableType(L, -1 - 1, depth + 1);
            } else {
                __PrintNormalType(L, -1, 1);
            }
        }
        lua_pop(L, 1);
        index++;
    }
    DEPTH_PRINTF(depth, "}\n");
}


static int UserPrint(lua_State* L){
    int num_args = lua_gettop(L);
    for( int i = 1; i <= num_args; i++ ){
        if( lua_istable(L, i) ){
            // 若为 table 
            __PrintTableType(L, i, 1);
        } else {
            // 若不是 table 
            __PrintNormalType(L, i, 1);
        }
        DEPTH_PRINTF(1, " ");
    }
    printf("\n");
    return 0;
}


lua_State* InitLuaState(){
    lua_State* ret = luaL_newstate();
    // loading base lib
    luaL_openlibs(ret);
    luaopen_cjson(ret);
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