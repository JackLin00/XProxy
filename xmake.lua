add_rules("mode.debug", "mode.release")
set_warnings("all", "error")

add_requires("libhv")

option("EnableXProxysLua")
    set_default(false)
    set_showmenu(true)
    set_category("option")
    set_description("Enable or disable Lua In XProxys.")
option_end()

option("EnableXProxycLua")
    set_default(false)
    set_showmenu(true)
    set_category("option")
    set_description("Enable or disable Lua In XProxyc.")
option_end()

-- common area   begin

-- disable some warn
add_cxflags("-Wno-sign-compare")
add_defines("_GLIBCXX_USE_SCHED_YIELD", "_GLIBCXX_USE_NANOSLEEP")

if is_mode("debug") then
    -- add_defines("DEBUG_FLAG")
end

-- add file 
add_files("src/cmd_codec.cpp", "src/protocol.cpp", "src/user_log.cpp")

-- add include 
add_includedirs("src/spdlog-1.0.0/include", "src")
-- add package
add_packages("libhv")


if has_config("EnableXProxysLua") or has_config("EnableXProxycLua") then
    add_requires("lua")
end
-- common area   end


target("XProxys")
    set_kind("binary")
    add_files("src/server.cpp", "src/server_module/*.cpp")
    add_includedirs("src/server_module")
    set_languages("cxx17")
    if has_config("EnableXProxysLua") then
        add_packages("lua")
        add_defines("ENABLE_LUA")
        add_defines("USE_INTERNAL_FPCONV", "NDEBUG")
        add_files("src/lua_module/lua-cjson/*.c|fpconv.c")
        add_files("src/server_module/lua_module_s.c")
    end


target("XProxyc")
    set_kind("binary")
    add_files("src/client.cpp", "src/client_module/*.cpp")
    add_includedirs("src/client_module")
    set_languages("cxx11")                                    -- 客户端这边现阶段需要尽量使用较低的 c++ 标准,以兼容尽可能的设备
    if has_config("EnableXProxycLua") then
        add_packages("lua")
        add_defines("ENABLE_LUA")
    end
