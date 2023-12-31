#include <iostream>
#include <cstdio>
#include "hv/hv.h"
#include "hv/hmain.h"
#include "hv/iniparser.h"
#include "server_loader.h"


static const char options[] = "hvc:";
// long options
static const option_t long_options[] = {
    {'h', "help",       NO_ARGUMENT},
    {'v', "version",    NO_ARGUMENT},
    {'c', "confile",    REQUIRED_ARGUMENT},
#ifdef ENABLE_LUA
    {'l', "lua",        REQUIRED_ARGUMENT}
#endif
};

#ifdef ENABLE_LUA
static const char detail_options[] = R"(
  -h|--help                 Print this information
  -v|--version              Print version
  -c|--confile <confile>    Set configure file
  -l|--lua <luafile>        run lua script file
)";
#else
static const char detail_options[] = R"(
  -h|--help                 Print this information
  -v|--version              Print version
  -c|--confile <confile>    Set configure file
)";
#endif

void PrintHelp() {
    printf("Usage: %s [%s]\n", "XProxys", options);
    printf("Options:\n%s\n", detail_options);
}

void PrintVersion() {
    printf("%s version %s\n", "XProxys", "V0.1");
}

int main(int argc, char** argv) {
    IniParser conf_file_handle;
    ServerLoaderParam_t load_param;
    hlog_set_file("XproxyS");
    int ret = parse_opt_long(argc, argv, long_options, ARRAY_SIZE(long_options));
    if (ret != 0) {
        PrintHelp();
        exit(ret);
    }

    if (get_arg("h")) {
        PrintHelp();
        exit(0);
    }

    // version
    if (get_arg("v")) {
        PrintVersion();
        exit(0);
    }

    if( get_arg("c") == NULL && get_arg("l") == NULL ){
        printf("please input config file path or do lua script\n");
        PrintHelp();
        exit(0);
    }

    load_param.doluafile_path = get_arg("l");

    if( load_param.doluafile_path == NULL ) {
        if (conf_file_handle.LoadFromFile(get_arg("c")) != 0) {
            printf("Parse config file[%s] error,please check it.\n", get_arg("c"));
            exit(0);
        }
    }

    load_param.ini_parser = &conf_file_handle;

    ServerLoader(&load_param);
    return 0;
}