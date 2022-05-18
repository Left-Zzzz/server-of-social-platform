/*************************************************************************
	> File Name: RedisManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 19 Mar 2022 09:50:29 PM CST
 ************************************************************************/

#ifndef _REDISMANAGER_H
#define _REDISMANAGER_H
#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "common/ret_value.h"

class RedisManager
{
public:
    redisReply* _reply;
    ~RedisManager();
    bool connect(std::string host, int port);
    bool connect(std::string host, int port, std::string passwd);
    std::string get(std::string key, int &reply_type);
    int set(std::string key, std::string value);
    int del(std::string key);
    int expire(std::string key, std::string value);
    static RedisManager* GetInstance();
    int setnx(std::string key, std::string value, int expire_time);
    int setnx(std::string key);

private:
    static RedisManager *instance_;
    RedisManager();
    RedisManager(RedisManager&) = delete;
    RedisManager &operator=(RedisManager&) = delete;
    redisContext* _connect;

};
#endif
