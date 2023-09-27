#ifndef __USER_LOG_H__
#define __USER_LOG_H__

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class DefaultLogger{
    private:
        static DefaultLogger handle;
        std::shared_ptr<spdlog::logger> logger;
        DefaultLogger() : logger(spdlog::stdout_color_mt("defaultLogger")) {}
        ~DefaultLogger(){}

    public:
        static spdlog::logger* GetLoggerHandle(){
            return handle.GetLogger();
        }

        spdlog::logger* GetLogger(){
            return logger.get();
        }
};


#define     TRACE(format, ...)            DefaultLogger::GetLoggerHandle()->trace(format , ##__VA_ARGS__)
#define     DEBUG(format, ...)            DefaultLogger::GetLoggerHandle()->debug(format , ##__VA_ARGS__)
#define     INFO(format, ...)             DefaultLogger::GetLoggerHandle()->info(format , ##__VA_ARGS__)
#define     WARN(format, ...)             DefaultLogger::GetLoggerHandle()->warn(format , ##__VA_ARGS__)
#define     ERROR(format, ...)            DefaultLogger::GetLoggerHandle()->error(format , ##__VA_ARGS__)
#define     SET_LOG_LEVEL(level)          DefaultLogger::GetLoggerHandle()->set_level(level)

#endif