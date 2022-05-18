/*************************************************************************
	> File Name: LogManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Mon 31 Jan 2022 04:36:24 PM CST
 ************************************************************************/

#ifndef _LOGMANAGER_H
#define _LOGMANAGER_H
class LogManager
{
private:
    FILE *log_fp_;
    char prefix[64];
    static LogManager *instance_;
    LogManager();
    LogManager(const char *file_path);
    LogManager(LogManager&) = delete;
    LogManager &operator=(LogManager&) = delete;
public:
    static LogManager *GetInstance();
    void log_fp_init(const char *prefix, const char *file_path);
    void log_fp_init(const char *file_path);
    void println(const char* format,  ...);
};
#endif
