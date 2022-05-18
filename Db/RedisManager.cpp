/*************************************************************************
	> File Name: RedisManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 19 Mar 2022 09:51:08 PM CST
 ************************************************************************/

#include "RedisManager.h"  

RedisManager *RedisManager::instance_ = NULL;

RedisManager::RedisManager()
{
    this->_connect = NULL;  
    this->_reply = NULL;               
};  
  
RedisManager::~RedisManager()  
{  
    this->_connect = NULL;  
    this->_reply = NULL;               
}  

RedisManager *RedisManager::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new RedisManager();
    //写死了，以后再配置吧
    instance_ -> connect("127.0.0.1", 6379, "left1234");
    return instance_;
}

bool RedisManager::connect(std::string host, int port, std::string passwd)
{
    if(!connect(host, port)) return 0;
    redisCommand(_connect, "AUTH left1234");
    return 1;

}

bool RedisManager::connect(std::string host, int port)  
{  
    this->_connect = redisConnect(host.c_str(), port);  
    if(this->_connect != NULL && this->_connect->err)  
    {  
        printf("connect error: %s\n", this->_connect->errstr);  
        return 0;  
    }  
    return 1;  
}  

std::string RedisManager::get(std::string key, int &reply_type)  
{  
    if(this -> _reply) freeReplyObject(this->_reply); 
    this->_reply = (redisReply*)redisCommand(this->_connect, "GET %s", key.c_str());  
    std::string str;
    if(this->_reply->str) str = this->_reply->str;
    reply_type = this -> _reply -> type;
    return str;  
}

int RedisManager::set(std::string key, std::string value)  
{  
    if(this -> _reply) freeReplyObject(this->_reply); 
    this->_reply = (redisReply*)redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());  
    int ret = REDIS_COMMOND_FAILED;
    if(_reply && strcmp(_reply -> str, "OK")) 
        ret = SUCCESS;
    return ret;
}  

//设置互斥锁
int RedisManager::setnx(std::string key)
{
    return setnx(key, "1", 10);
}


int RedisManager::setnx(std::string key, std::string value, int expire_time)
{
    if(this -> _reply) freeReplyObject(this->_reply); 
    this -> _reply = (redisReply*)redisCommand(this -> _connect, "SETNX %s %s", 
                                             key.c_str(), value.c_str());
    int ret = REDIS_COMMOND_FAILED;
    if(_reply && _reply -> type == REDIS_REPLY_INTEGER) 
        ret = _reply -> integer;
    if(ret <= 0) return REDIS_COMMOND_FAILED;
    ret = expire(key, std::to_string(expire_time));
    return ret;
}

int RedisManager::del(std::string key)  
{  
    if(this -> _reply) freeReplyObject(this->_reply); 
    this -> _reply = (redisReply*)redisCommand(this->_connect, "DEL %s", key.c_str());  
    int ret = REDIS_COMMOND_FAILED;
    if(_reply && _reply -> type == REDIS_REPLY_INTEGER) 
        ret = _reply -> integer;
    if(ret == 1) ret = SUCCESS;
    else ret = REDIS_COMMOND_FAILED;
    return ret;
}  

int RedisManager::expire(std::string key, std::string value)  
{  
    if(this -> _reply) freeReplyObject(this->_reply); 
    this -> _reply = (redisReply*)redisCommand(this->_connect, 
                                               "EXPIRE %s %s", 
                                               key.c_str(), value.c_str());  
    int ret = REDIS_COMMOND_FAILED;
    if(_reply && _reply -> type == REDIS_REPLY_INTEGER) 
        ret = _reply -> integer;
    if(ret == 1) ret = SUCCESS;
    else ret = REDIS_COMMOND_FAILED;
    return ret;
}  
