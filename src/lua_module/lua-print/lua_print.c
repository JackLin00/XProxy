#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define      DEPTH_PRINTF(depth, ...)                           do{                                                      \
                                                                    for(int i = 0; i < depth; i++)         printf("  "); \
                                                                    printf(__VA_ARGS__);                                 \
                                                                }while(0)

static void __PrintTableType(lua_State* L, int idx, int depth);
static void __PrintNormalType(lua_State* L, int idx, int depth, int in_table){
    switch(lua_type(L, idx)){
        case LUA_TSTRING:
            if(in_table)            DEPTH_PRINTF(depth, "\e[31m\"%s\"\e[0m", lua_tostring(L, idx));
            else                    DEPTH_PRINTF(depth, "\"%s\"", lua_tostring(L, idx));
            break;
        case LUA_TBOOLEAN:
            DEPTH_PRINTF(depth, lua_toboolean(L, idx) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            if(in_table)            DEPTH_PRINTF(depth, "\e[32m%g\e[0m", lua_tonumber(L, idx));
            else                    DEPTH_PRINTF(depth, "%g", lua_tonumber(L, idx));
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
    char pre_print_table = 0;
    lua_pushnil(L);
    DEPTH_PRINTF(depth, "{\n");
    while( lua_next(L, idx) ){
        if( index != 0 )               printf(",\n");
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if( lua_isinteger(L, -2) ){
            // int 作为索引
            __PrintNormalType(L, -1, depth + 1, 1);
            pre_print_table = 0;
        } else {
            // 用字符作为键
            DEPTH_PRINTF(depth, "\e[31m%s\e[0m = ", lua_tostring(L, -2));
            if( lua_istable(L, -1) ){
                // 若为 table 则递归
                __PrintTableType(L, -1 - 1, depth + 1);
                pre_print_table = 1;
            } else {
                __PrintNormalType(L, -1, 1, 1);
                pre_print_table = 0;
            }
        }
        lua_pop(L, 1);
        index++;
    }
    printf("\n");
    DEPTH_PRINTF(depth, "}");
    if( pre_print_table == 1 )   printf("\n");

}


int UserPrint(lua_State* L){
    int num_args = lua_gettop(L);
    for( int i = 1; i <= num_args; i++ ){
        if( lua_istable(L, i) ){
            // 若为 table 
            __PrintTableType(L, i, 0);
            printf("\n");
        } else {
            // 若不是 table 
            __PrintNormalType(L, i, 0, 0);
        }
        DEPTH_PRINTF(0, " ");
    }
    printf("\n");
    return 0;
}