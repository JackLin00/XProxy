#include <iostream>
#include "hv/hv.h"
#include "hv/hmain.h"
#include "hv/iniparser.h"
#include "client_loader.h"

static const char options[] = "hvc:";
// long options
static const option_t long_options[] = {
    {'h', "help",       NO_ARGUMENT},
    {'v', "version",    NO_ARGUMENT},
    {'c', "confile",    REQUIRED_ARGUMENT}
};

static const char detail_options[] = R"(
  -h|--help                 Print this information
  -v|--version              Print version
  -c|--confile <confile>    Set configure file
)";


void PrintHelp() {
    printf("Usage: %s [%s]\n", "XProxyc", options);
    printf("Options:\n%s\n", detail_options);
}

void PrintVersion() {
    printf("%s version %s\n", "XProxyc", "V0.1");
}


int main(int argc, char** argv) {
    IniParser conf_file_handle;
    ClientLoaderParam_t load_param; 
    hlog_set_file("XproxyC");
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

    if( get_arg("c") == NULL ){
        printf("please input config file path\n");
        PrintHelp();
        exit(0);
    }

    if (conf_file_handle.LoadFromFile(get_arg("c")) != 0) {
        printf("Parse config file error,please check it.\n");
        exit(0);
    }

    load_param.ini_parser = &conf_file_handle;

    ClientLoader(&load_param);

    return 0;
}
