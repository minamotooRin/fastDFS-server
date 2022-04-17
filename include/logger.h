#ifndef _LOGGER_H
#define _LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO // 输出文件名和行号
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#define ERROR   1
#define WARNING 2
#define INFO    3
#define DEBUG   4

//以下宏spdlog定义， 日志输出到控制台
//SPDLOG_DEBUG(logger,...)
//SPDLOG_INFO(logger,...)
//SPDLOG_WARN(logger,...)
//SPDLOG_ERROR(logger,...)
//SPDLOG_CRITICAL(logger,...)

//以下宏spdlog定义， 日志输出到旋转日志文件
//SPDLOG_LOGGER_DEBUG(logger,...)
//SPDLOG_LOGGER_INFO(logger,...)
//SPDLOG_LOGGER_WARN(logger,...)
//SPDLOG_LOGGER_ERROR(logger,...)
//SPDLOG_LOGGER_CRITICAL(logger,...)

#endif