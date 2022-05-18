/*************************************************************************
	> File Name: MessageManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 16 Jan 2022 03:11:46 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"MessageManager.h"
#include"DbManager.h"
#include"RelationManager.h"
#include"PhotoManager.h"
#include"UserManager.h"
#include"LogManager.h"

static LogManager *log_mgr = LogManager::GetInstance();
DbManager db_svr;

MessageManager::MessageManager()
{
    Start();
}

MessageManager::~MessageManager()
{
    Shutdown();
}

void MessageManager::Start()
{
    log_mgr = LogManager::GetInstance();
    log_mgr -> log_fp_init("message.log");
    //初始化DB，从配置文件里读连接
    int ret = db_svr.initDb();
    if(ret != SUCCESS)
    {
        log_mgr -> println("[MessMgr] db init failed. ret:[%d]\n", ret);
        exit(1);
    }
    else log_mgr -> println("[MessMgr] db init successed.\n");
    std::cout << "MessageManager Start.\n";
}

void MessageManager::Proc()
{
    std::cout << "MessageManager Proc.\n";
}
void MessageManager::Shutdown()
{
    std::cout << "MessageManager Shutdown.\n";
}

void MessageManager::Restart()
{
    std::cout << "MessageManager Restart.\n";
}

MessageInfo *MessageManager::GetMessage(int user_id, int message_id)
{
    int ret;
    //todo：删除消息时候，对应缓冲区MessageInfo设置为删除标志，防止消除消息后仍能获取消息。
    for(int i = 0; i < message_count_; i++)
    {
        if(messages_[i].message_id() == message_id)
        {
            if(messages_[i].publisher() != user_id)
            {
                return NULL;
            }
            return &messages_[i];
        }
    }
    //没有设计内存淘汰算法，缓存区满了就-1，待优化
    if(message_count_ >= 256) message_count_--;
    std::string Sql = "select * from tb_message where message_id = " + to_string(message_id) + " and user_id = " + to_string(user_id) + ";";
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS || db_svr.result == NULL) return NULL;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(db_svr.row == NULL) return NULL;
    //1.user_id,2.message_id,3.content,4.publish_time
    messages_[message_count_].set_publisher(atoi(db_svr.row[0]));
    messages_[message_count_].set_message_id(atoi(db_svr.row[1]));
    messages_[message_count_].set_content(db_svr.row[2]);
    messages_[message_count_].set_publish_time(atoi(db_svr.row[3]));
    mysql_free_result(db_svr.result);
    return &messages_[message_count_++];
}

int MessageManager::GetMessage(int message_id, MessageInfo &message_info)
{
    int ret;
    std::string Sql = "select * from tb_message where message_id = " + to_string(message_id) + ";";
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS) return ret;
    if(db_svr.result == NULL) return DB_QUERY_FAIL;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(db_svr.row == NULL) return DB_QUERY_FAIL;
    //1.user_id,2.message_id,3.content,4.publish_time
    message_info.set_publisher(atoi(db_svr.row[0]));
    message_info.set_message_id(atoi(db_svr.row[1]));
    message_info.set_content(db_svr.row[2]);
    message_info.set_publish_time(atoi(db_svr.row[3]));
    message_info.set_last_modify_time(atoi(db_svr.row[4]));
    log_mgr -> println("publisher:[%s], message_id:[%s], content[%s], publish_time:[%s], last_mod_time[%s]", 
           db_svr.row[0], db_svr.row[1],db_svr.row[2], db_svr.row[3], db_svr.row[4]);
    mysql_free_result(db_svr.result);
    
    return SUCCESS;
}

MessageInfo *MessageManager::GetMessage(int message_id)
{
    //窝草啥玩意
    int ret;
    //todo：删除消息时候，对应缓冲区MessageInfo设置为删除标志，防止消除消息后仍能获取消息。
    for(int i = 0; i < message_count_; i++)
    {
        if(messages_[i].message_id() == message_id)
            return &messages_[i];
    }
    //没有设计内存淘汰算法，缓存区满了就-1，待优化
    if(message_count_ >= 256) message_count_--;
    std::string Sql = "select * from tb_message where message_id = " + to_string(message_id) + ";";
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS || db_svr.result == NULL) return NULL;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(db_svr.row == NULL) return NULL;
    //1.user_id,2.message_id,3.content,4.publish_time
    messages_[message_count_].set_publisher(atoi(db_svr.row[0]));
    messages_[message_count_].set_message_id(atoi(db_svr.row[1]));
    messages_[message_count_].set_content(db_svr.row[2]);
    messages_[message_count_].set_publish_time(atoi(db_svr.row[3]));
    mysql_free_result(db_svr.result);
    return &messages_[message_count_++];
}

int MessageManager::PublishMessage(int publisher, std::string content, int publish_time, int &message_id)
{
    MessageInfo message;
    message.set_publisher(publisher);
    message.set_content(content);
    message.set_publish_time(publish_time);
    message.set_last_modify_time(publish_time);
    return PublishMessage(message, message_id);
}

int MessageManager::PublishMessage(MessageInfo message, int &message_id)
{
    int ret;
    
    //获取GlbMessageId
    message_id = GetGlbMessageId();
    if(message_id <= 0) return message_id;

    log_mgr -> println("message_id = %d", message_id);
    message.set_message_id(message_id);

    //转义message
    char escape_content[10240];
    mysql_real_escape_string(db_svr.conn, escape_content, message.content().c_str(), message.content().size());
    message.set_content(escape_content);
    //保存到数据库
    std::string Sql = "insert into tb_message values(" + to_string(message.publisher()) + ", " + to_string(message.message_id()) + ", \'" + message.content() + "\', " + to_string(message.publish_time()) + ", " + to_string(message.last_modify_time()) + ");";
    ret = db_svr.execSql(Sql);
    db_svr.freeRes();
    if(ret != SUCCESS) return ret;
    message_id = message_id;
    
    return SUCCESS;
}

int MessageManager::ModifyMessage(int user_id, int message_id, int modify_time, string &content)
{
    int ret;
    MessageInfo ret_message_info;
    //查询消息并进行用户校验
    ret = GetMessage(message_id, ret_message_info);
    if(ret != SUCCESS) 
        return ret;
    //用户权限校验：如果发布者和修改者用户id不一致则判断无权限
    //后续添加管理员组
    if(user_id != ret_message_info.publisher())
        return PERMISSION_DENINED;
    //更新信息
    std::string Sql = "update tb_message set last_modify_time = " + to_string(modify_time) + ", content = \"" + content + "\" where message_id =" + to_string(message_id) + ";";
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS) return ret;
    return SUCCESS;
}

int MessageManager::DeleteMessage(int user_id, int message_id)
{
    int ret;
    std::string Sql = "select * from tb_message where user_id = " + to_string(user_id) + " and message_id = " + to_string(message_id) + ";";
    db_svr.execSql(Sql);
    if(db_svr.result == NULL) return MESSAGE_NOT_EXIST;
    return DeleteMessage(message_id);
}

int MessageManager::DeleteMessage(int message_id)
{
    int ret;
    std::string Sql = "delete from tb_message where message_id = " + to_string(message_id) + ";";
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS) return ret;
    return SUCCESS;
}

//获取GlobalMessageId
int MessageManager::GetGlbMessageId()
{
    int ret, message_id = -1;
    std::string Sql = "select value from tb_var where id = 2;";
    ret = db_svr.execSql(Sql);
    if(ret == DB_QUERY_FAIL || db_svr.result == NULL) return ret;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(db_svr.row == NULL) return DB_QUERY_FAIL;
    message_id = atoi(db_svr.row[0]) + 1;
    mysql_free_result(db_svr.result);

    Sql = "update tb_var set value = value + 1 where id = 2;";
    ret = db_svr.execSql(Sql);
    if(ret == DB_QUERY_FAIL) return ret;
    log_mgr -> println("GetGlbMessageId success, GlbId = [%d]", message_id);
    return message_id;
}
