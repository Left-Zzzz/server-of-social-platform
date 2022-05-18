/*************************************************************************
	> File Name: MessageHandler.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Tus 29 Mar 2022 9:03:14 PM CST
 ************************************************************************/

#ifndef _MessageHandler_H
#define _MessageHandler_H

#include"MainHandler.h"
#include"Service/MessageService.h"
#include"MessageInfo.h"
class MessageHandler:virtual public MainHandler
{
//MessageHandler参数常量设置
private:
    //Redis messageid_lock前缀名
    const string REDIS_MESSAGE_LOCK_PREFIX = "ssp:lock:messageid:";
    //Redis key前缀名
    const string REDIS_MESSAGEID_PREFIX =  "ssp:messageid:";
    //MessageHandler路由地址
    const string SCRIPT_NAME_ = "/message";
    const string REDIS_MESSAGE_EXPIRETIME = "1800";
//单例模式设置
private:
    MessageHandler();
    ~MessageHandler();
    MessageHandler(MessageHandler&);
    MessageHandler operator=(MessageHandler&);
    static MessageHandler *instance_;
//子业务Handle入口
private:
    int PublishMessageHandle(int user_id, string &input_data_json);
    int GetMessageHandle(int user_id, stringkv &input_data_query);
    int ModifyMessageHandle(int user_id, string &input_data_json);
    //互斥锁解决缓存击穿
    int GetRedisMessageByMutex(int user_id, int message_id);
    //设置逻辑过期解决缓存击穿
    int GetRedisMessageBylogicExpire(int user_id, int message_id);
    //从数据库中获取消息
    int GetMessageFromDb(int user_id, int message_id, 
                         MessageInfo &ret_message_info);
//路由Handle入口
public:
    virtual int Handle(string &req_method, int user_id, string &input_data);
    virtual int Handle(string &req_method, int user_id, stringkv &input_data);
    virtual int Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query);
    static MessageHandler *GetInstance();
};
#endif
