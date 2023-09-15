add_rules("mode.debug", "mode.release")
set_warnings("all", "error")

add_requires("libhv")
set_languages("cxx11", "c99")


option("XProxysOpenLua")
    set_default(false)
    set_showmenu(true)
    set_category("option")
    set_description("Enable or disable Lua In XProxys.")
option_end()

option("XProxycOpenLua")
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
    add_defines("DEBUG_FLAG")
end

-- add file 
add_files("src/cmd_codec.cpp", "src/protocol.cpp", "src/user_log.cpp")

-- add include 
add_includedirs("src/spdlog-1.0.0/include", "src")
-- add package
add_packages("libhv")


if has_config("XProxysOpenLua") or has_config("XProxycOpenLua") then
    add_requires("lua")
end
-- common area   end


target("XProxys")
    set_kind("binary")
    add_files("src/server.cpp", "src/server_module/*.cpp")
    add_includedirs("src/server_module")
    if has_config("XProxysOpenLua") then
        add_packages("lua")
    end


target("XProxyc")
    set_kind("binary")
    add_files("src/client.cpp", "src/client_module/*.cpp")
    add_includedirs("src/client_module")
    if has_config("XProxycOpenLua") then
        add_packages("lua")
    end
