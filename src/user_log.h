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


#define     TRACE(...)            DefaultLogger::GetLoggerHandle()->trace(__VA_ARGS__)
#define     DEBUG(...)            DefaultLogger::GetLoggerHandle()->debug(__VA_ARGS__)
#define     INFO(...)             DefaultLogger::GetLoggerHandle()->info(__VA_ARGS__)
#define     WARN(...)             DefaultLogger::GetLoggerHandle()->warn(__VA_ARGS__)
#define     ERROR(...)            DefaultLogger::GetLoggerHandle()->error(__VA_ARGS__)
#define     SET_LOG_LEVEL(level)  DefaultLogger::GetLoggerHandle()->set_level(level)

#endif