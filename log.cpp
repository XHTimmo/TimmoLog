#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "log.h"

static LogLevel g_log_level = LOG_INFO;
static FILE* log_file = NULL;              // 日志文件指针

// ANSI颜色代码
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"  // 青色（时间戳）

// 日志等级字符串
static const char* log_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

// 日志等级对应的颜色
static const char* log_level_colors[] = {
    ANSI_COLOR_BLUE,    // DEBUG - 蓝色
    ANSI_COLOR_GREEN,   // INFO - 绿色
    ANSI_COLOR_YELLOW,  // WARN - 黄色
    ANSI_COLOR_RED,     // ERROR - 红色
    ANSI_COLOR_MAGENTA  // FATAL - 洋红色
};

// 初始化日志等级
void log_init(LogLevel level, const char* filename) {
    g_log_level = level;

    // 关闭已打开的日志文件
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }

    // 打开新的日志文件（追加模式）
    if (filename != NULL) {
        log_file = fopen(filename, "w");
        if (log_file == NULL) {
            fprintf(stderr, "无法打开日志文件: %s\n", filename);
        }
    }

}

// 关闭日志文件
void log_close(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

// 通用日志打印函数
static void log_print(LogLevel level, const char *format, va_list args) {
    // 如果当前日志等级低于设置的阈值，则不打印
    if (level < g_log_level) {
        return;
    }

    // 获取当前时间
    time_t now;
    time(&now);
    struct tm *local_time = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    va_list args_terminal;  // 用于终端输出的副本
    va_list args_file;      // 用于文件输出的副本
    va_copy(args_terminal, args);  // 复制原始args（未被消耗）
    va_copy(args_file, args);      // 复制原始args（未被消耗）

    // 打印到终端（带颜色）仅打印3级以上
    if(level >= LOG_WARN){
      // 打印日志头部（时间 + 等级）
      fprintf(stderr, "%s %s %s [%s]:\t",
              ANSI_COLOR_CYAN, time_str, log_level_colors[level], log_level_strings[level]);
      // 打印日志内容
      vfprintf(stderr, format, args_terminal);
      // 确保每条日志都换行
      fprintf(stderr, "%s\n",ANSI_COLOR_RESET);
    }

    // 打印到文件（带颜色，适合支持ANSI的查看器）
    if (log_file != NULL) {
      fprintf(log_file, "%s%s%s [%s]: ", 
              ANSI_COLOR_CYAN, time_str, 
              log_level_colors[level], log_level_strings[level]);
      vfprintf(log_file, format, args_file);
      fprintf(log_file, "%s\n", ANSI_COLOR_RESET);
      fflush(log_file);  // 立即刷新，确保日志写入
      va_end(args_terminal);  // 释放复制的参数
      va_end(args_file);  // 释放复制的参数
    }

    // 如果是致命错误，退出程序
    if (level == LOG_FATAL) {
        log_close();
        exit(EXIT_FAILURE);
    }
}

// 各等级日志的具体实现
void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_DEBUG, format, args);
    va_end(args);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_INFO, format, args);
    va_end(args);
}

void log_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_WARN, format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_ERROR, format, args);
    va_end(args);
}

void log_fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_print(LOG_FATAL, format, args);
    va_end(args);
}