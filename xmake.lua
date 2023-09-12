add_rules("mode.debug", "mode.release")
set_warnings("all", "error")

add_requires("libhv")
set_languages("cxx11", "c99")


rule("MoveConfFile")
    after_build(function (target)
        -- os.cp(os.projectdir() .. "/config/*", target:targetdir())
    end)

target("XProxys")
    set_kind("binary")
    add_files("src/server.cpp", "src/server_module/*.cpp", "src/protocol.cpp")
    add_files("src/cmd_codec.cpp")
    add_packages("libhv")
    add_includedirs("src/server_module", "src")
    add_defines("_GLIBCXX_USE_SCHED_YIELD", "_GLIBCXX_USE_NANOSLEEP")
    add_rules("MoveConfFile")
    if is_mode("debug") then
        add_defines("DEBUG_FLAG")
    end


target("XProxyc")
    set_kind("binary")
    add_files("src/client.cpp", "src/client_module/*.cpp", "src/protocol.cpp")
    add_files("src/cmd_codec.cpp")
    add_includedirs("src/client_module", "src")
    add_packages("libhv")
    add_rules("MoveConfFile")
    if is_mode("debug") then
        add_defines("DEBUG_FLAG")
    end
