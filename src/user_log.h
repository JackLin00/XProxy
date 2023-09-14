#ifndef __USER_LOG_H__
#define __USER_LOG_H__

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

static std::shared_ptr<spdlog::logger> default_logger = spdlog::stdout_color_mt("defaultLogger");

#define     TRACE(...)        default_logger->trace(__VA_ARGS__)
#define     DEBUG(...)        default_logger->debug(__VA_ARGS__)
#define     INFO(...)         default_logger->info(__VA_ARGS__)
#define     WARN(...)         default_logger->warn(__VA_ARGS__)
#define     ERROR(...)        default_logger->error(__VA_ARGS__)

#endif