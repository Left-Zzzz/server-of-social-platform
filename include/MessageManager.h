/*************************************************************************
	> File Name: MessageManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 03:42:45 PM CST
 ************************************************************************/

#ifndef _MESSAGEMANAGER_H
#define _MESSAGEMANAGER_H
#include"MessageInfo.h"
class MessageManager
{
private:
    MessageInfo messages_[256];
    GETSETVAR(int, message_count)
public:
    void Start();
    void Proc();
    void Shutdown();
    void Restart();
public:
    MessageManager();
    ~MessageManager();
    MessageInfo *GetMessage(int user_id, int message_id);
    MessageInfo *GetMessage(int message_id);
    int GetMessage(int message_id, MessageInfo &message_info);
    int PublishMessage(int publisher, std::string content, int publish_time, int &message_id);
    int PublishMessage(MessageInfo message, int &message_id);
    int ModifyMessage(int user_id, int message_id, int modify_time, string &content);
    int DeleteMessage(int user_id, int message_id);
    int DeleteMessage(int message_id);
    int GetGlbMessageId();
};
#endif
