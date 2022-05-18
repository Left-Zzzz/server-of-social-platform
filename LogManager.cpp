/*************************************************************************
	> File Name: LogManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Mon 31 Jan 2022 04:35:19 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include<sys/file.h>
#include"common/macro.h"
#include"LogManager.h"
using namespace std;

LogManager *LogManager::instance_ = NULL;

LogManager *LogManager::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new LogManager();
    return instance_;
}

void LogManager::log_fp_init(const char *file_path)
{
    log_fp_ = fopen(file_path, "a+");
    if(!log_fp_) myperror("fopen");
}

void LogManager::log_fp_init(const char *prefix, const char *file_path)
{
    strcpy(this -> prefix, prefix);
    log_fp_ = fopen(file_path, "a+");
    if(!log_fp_) myperror("fopen");
}

//空实现,以后有需求在补充
LogManager::LogManager()
{

}

void LogManager::println(const char* format,  ...)
{
    if(!log_fp_)
    {
        fprintf(stderr, "log_fp_不存在%p！\n", log_fp_);
        return;
    }
    char buf[1024];
    char* p = buf;
    va_list args;
    va_start(args, format);
    vsprintf(p, format, args);
    va_end(args);
    fprintf(log_fp_, "%s\n", buf);
    fflush(log_fp_);
}

LogManager::LogManager(const char *file_path)
{
    log_fp_init(file_path);
}
