/*************************************************************************
	> File Name: ResponseManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 29 Jan 2022 04:40:41 PM CST
 ************************************************************************/

#ifndef _RESPONSEMANAGER_H
#define _RESPONSEMANAGER_H
#include<vector>
#include<string>
#include"common/macro.h"
class ResponseManager
{
    GETSETVAR(std::string, status)
    GETSETVAR(std::string, content_type)
    GETSETVAR(std::string, content)
private:
    std::vector<std::string> cookies_;
    static ResponseManager *instance_;
    ResponseManager(const ResponseManager&) = delete;//禁止拷贝构造
    ResponseManager &operator=(const ResponseManager) = delete;//禁止赋值
    ResponseManager();
public:
    static ResponseManager* GetInstance();
    int set_cookie(std::string name, std::string &value,\
            time_t expire = 0, int max_age = 3600, std::string &&path = "/");
    std::string GetResponse();
    ~ResponseManager();
};

#endif
