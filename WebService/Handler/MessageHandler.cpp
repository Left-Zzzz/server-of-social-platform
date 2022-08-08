/*************************************************************************
	> File Name: MessageHandler.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Feb 29 Mar 2022 06:51:17 PM CST
 ************************************************************************/

#include"Handler/MessageHandler.h"
#include"Service/MessageService.h"

MessageHandler *MessageHandler::instance_ = new MessageHandler();

MessageHandler *MessageHandler::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new MessageHandler();
    return instance_;
}

MessageHandler::MessageHandler()
{
    //往MainHandler的handler_list中注册script
    handler_list[SCRIPT_NAME_] = this;
    auth_script_list.insert(SCRIPT_NAME_);
    if(handler_list.count(SCRIPT_NAME_))
        log_mgr -> println("[MessageHandler]:注册成功！");
    else log_mgr -> println("[MessageHandler]:注册失败！");
}

MessageHandler::~MessageHandler()
{
    //往MainHandler的handler_list中删除script
    auto it = MainHandler::handler_list.find(SCRIPT_NAME_);
    if(it != MainHandler::handler_list.end())
        MainHandler::handler_list.erase(it);
}

//如果是非GET请求，如果没有URL参数但有消息体，则走这个处理接口
int MessageHandler::Handle(string &req_method, int user_id, string &input_data)
{
    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
    return REQ_METHOD_NOT_EXIST;
}

//处理请求
int MessageHandler::Handle(string &req_method, int user_id, stringkv &input_data)
{
    log_mgr -> println("[MessageHandler]:user_id=[%d]", user_id);
    if(req_method != "GET")
    {
        rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
        return REQ_METHOD_NOT_EXIST;
    }
    return GetMessageHandle(user_id, input_data);
}
//如果是非GET请求，如果有URL参数和消息体，则走这个处理接口
int MessageHandler::Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query)
{
    log_mgr -> println("[MessageHandler]:user_id=[%d]", user_id);
    if(req_method == "POST")
    {
        return PublishMessageHandle(user_id, input_data_body);
    }
    //PUT方法暂时不可用
    else if(req_method == "PUT")
    {
        return ModifyMessageHandle(user_id, input_data_body);
    }
    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
    return REQ_METHOD_NOT_EXIST;
}

int MessageHandler::GetMessageHandle(int user_id, stringkv &input_data_query)
{
    rsp_mgr = ResponseManager::GetInstance();
    RedisManager *redis_mgr =  RedisManager::GetInstance();
    string ret_json;
    int ret;
    //检查参数合法性
    if(!input_data_query.count("message_id") || !CheckIsDigit(input_data_query["message_id"]))
    {
        rsp_mgr -> set_content(get_json(QUERY_MESSAGE_ERR, "请求的信息错误", ""));
        rsp_mgr -> set_status(to_string(400));
        return QUERY_MESSAGE_ERR;
    }
    int message_id;
    sscanf(input_data_query["message_id"].c_str(), "%d", &message_id);

    //互斥锁解决缓存击穿
    GetRedisMessageByMutex(user_id, message_id);
    //设置逻辑过期解决缓存击穿
    //GetRedisMessageBylogicExpire();
    //【旧版】在缓存查找是否有数据
    /*
    ret_json = redis_mgr -> get(REDIS_MESSAGEID_PREFIX + to_string(message_id), ret);
    if(ret != REDIS_REPLY_NIL)
    {
        if(ret_json == "")
        {
            rsp_mgr -> set_content(get_json(MESSAGE_NOT_EXIST,
                                            "请求的消息不存在（从缓存中获取的消息）", ""));
            return MESSAGE_NOT_EXIST;
        }
        rsp_mgr -> set_content(get_json(SUCCESS, "成功(从缓存中获取的消息)", string(ret_json)));
        return SUCCESS;
    }
    */
    //若缓存中没有数据，则进行下一步操作。
    //将得到的pb数据转换成对象数据，再将对象数据转换成json数据
    //将数据添加到缓存中,下次查看直接从缓存中读。
    return SUCCESS;
}

int MessageHandler::PublishMessageHandle(int user_id, string &input_data_body)
{
    int now = time(0);
    ssp::ReqInfoBase req_base;
    ssp::RspInfoBase rsp_base;
    char send_buf[10240] = {}, recv_buf[10240] = {};
    Document doc;
    doc.Parse<kParseStopWhenDoneFlag>(input_data_body.c_str());
    int ret;
    //判断消息内容合法性
    if(!doc.IsObject() || !doc.HasMember("content") || !doc["content"].IsString())
    {
        rsp_mgr -> set_content(get_json(QUERY_MESSAGE_ERR, "请求内容非法！", ""));
        return QUERY_MESSAGE_ERR;
    }
    //发送消息，返回消息id
    ssp::PublishMessageReq *publish_mess_req = req_base.mutable_publish_mess_req();
    publish_mess_req -> set_user_id(user_id);
    publish_mess_req -> set_content(doc["content"].GetString());
    publish_mess_req -> set_publish_time(now);
    req_base.set_mess_type(PUBLISH_MESSAGE_REQ);
    req_base.SerializeToArray(send_buf, 10239);
    //从bus_mgr中获取sendbuf
    ret = bus_mgr -> SendMessage("message_write", send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息失败", ""));
        log_mgr -> println("消息服务器SendMessage失败");
        return ret;
    }
    ret = bus_mgr -> TryGetMessage("message_read", recv_buf, 10239, 10000, 500);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "服务器响应超时！消息可能发送成功了，但是未推送给好友。", ""));
        log_mgr -> println("关系服务器TryGetMessage失败");
        return ret;
    }
    rsp_base.ParseFromArray(recv_buf, 10239);
    ssp::PublishMessageRsp *publish_mess_rsp = rsp_base.mutable_publish_mess_rsp();
    int message_id = publish_mess_rsp -> ret();
    if(message_id <= 0)
    {
        ret = message_id;
        rsp_mgr -> set_content(get_json(ret, "发送消息失败", ""));
        log_mgr -> println("消息服务器TryGetMessage失败,ret = [%d]", ret);
        return ret;
    }

    //删除缓存
    RedisManager *redis_mgr = RedisManager::GetInstance();
    redis_mgr -> del(REDIS_MESSAGEID_PREFIX + to_string(message_id));

    //获取好友列表
    req_base.Clear();
    rsp_base.Clear();
    memset(send_buf, 0, sizeof(send_buf));
    memset(recv_buf, 0, sizeof(recv_buf));
    ssp::GetFriendsReq *get_friends_req = req_base.mutable_get_friends_req();
    req_base.set_mess_type(GET_FRIENDS_REQ);
    get_friends_req -> set_user_id(user_id);
    req_base.SerializeToArray(send_buf, 10239);
    req_base.set_mess_type(0);
    req_base.ParseFromArray(send_buf, 10239);
    log_mgr -> println("mess_type : %d\n", req_base.mess_type());
    log_mgr -> println("parse mess_type %d", req_base.mess_type());
    log_mgr -> println("ret_size = [%d]", req_base.ByteSizeLong());
    ret = bus_mgr -> SendMessage("relation_write", send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息成功，但消息未能推送给好友", ""));
        log_mgr -> println("关系服务器SendMessage失败");
        return ret;
    }
    ret = bus_mgr -> TryGetMessage("relation_read", recv_buf, 10239, 10000, 500);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息成功，但消息未能推送给好友", ""));
        log_mgr -> println("关系服务器TryGetMessage失败");
        return ret;
    }
    rsp_base.ParseFromArray(recv_buf, 10239);
    ssp::GetFriendsRsp *get_friends_rsp = rsp_base.mutable_get_friends_rsp();
    ret = get_friends_rsp -> ret();
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息成功，但消息未能推送给好友", ""));
        log_mgr -> println("获取好友列表失败, ret = %d", ret);
        return ret;
    }
    //创建快照
    req_base.Clear();
    memset(send_buf, 0, sizeof(send_buf));
    memset(recv_buf, 0, sizeof(recv_buf));
    ssp::CreatePhotoReq *create_photo_req = req_base.mutable_create_photo_req();
    for(int i = 0; i < get_friends_rsp -> friend_id_size(); i++)
    {
        create_photo_req -> add_user_id(get_friends_rsp -> friend_id(i));
    }
    //这里rsp_base还保存着RelationSvr返回的好友id，不能提前Clear
    rsp_base.Clear();
    create_photo_req -> set_message_id(message_id);
    create_photo_req -> set_publisher_id(user_id);
    create_photo_req -> set_time(now);
    req_base.set_mess_type(CREATE_PHOTO_REQ);
    req_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage("photo_write", send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "send_buf:服务器忙", ""));
        return ret;
    }
    ret = bus_mgr -> TryGetMessage("photo_read", recv_buf, 10239, 10000, 500);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息成功，但未能成功推送给好友", ""));
        log_mgr -> println("关系服务器TryGetMessage失败");
        return ret;
    }
    rsp_base.ParseFromArray(recv_buf, 10239);
    ssp::CreatePhotoRsp *create_photo_rsp = rsp_base.mutable_create_photo_rsp();
    ret = get_friends_rsp -> ret();
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "发送消息成功，但消息未能推送给好友", ""));
        log_mgr -> println("创建快照失败");
        return ret;
    }
    rsp_mgr -> set_content(get_json(ret, "成功", ""));
    return SUCCESS;
}

int MessageHandler::ModifyMessageHandle(int user_id, string &input_data_body)
{
    log_mgr -> println("modify_message_handle.");
    int now = time(0);
    ssp::ReqInfoBase req_base;
    ssp::RspInfoBase rsp_base;
    char send_buf[10240] = {}, recv_buf[10240] = {};
    Document doc;
    doc.Parse<kParseStopWhenDoneFlag>(input_data_body.c_str());
    int ret;
    //判断消息内容合法性
    if(!doc.IsObject()
       || !doc.HasMember("content") || !doc["content"].IsString()
       || !doc.HasMember("message_id") || !doc["message_id"].IsInt())
    {
        rsp_mgr -> set_content(get_json(QUERY_MESSAGE_ERR, "请求内容非法！", ""));
        return QUERY_MESSAGE_ERR;
    }
    //通过bug_mgr向消息服务器发送modifymessage请求
    req_base.set_mess_type(MODIFY_MESSAGE_REQ);
    ssp::ModifyMessageReq *modify_mess_req = 
    req_base.mutable_modify_mess_req();
    modify_mess_req -> set_user_id(user_id);
    modify_mess_req -> set_message_id(doc["message_id"].GetInt());
    modify_mess_req -> set_content(doc["content"].GetString());
    modify_mess_req -> set_modify_time(now);
    req_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage("message_write", send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "修改消息失败", ""));
        log_mgr -> println("关系服务器SendMessage失败");
        return ret;
    }
    //尝试获取消息服务器回复
    ret = bus_mgr -> TryGetMessage("message_read", recv_buf, 10239, 10000, 500);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "修改消息失败", ""));
        log_mgr -> println("关系服务器TryGetMessage失败");
        return ret;
    }
    //如果结果为失败，返回状态码
    ssp:ModifyMessageRsp *modify_mess_rsp = rsp_base.mutable_modify_mess_rsp();
    ret = modify_mess_rsp -> ret();
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "修改消息失败", ""));
        return ret;
    }
    //如果成功，尝试在Redis中删除缓存
    RedisManager *redis_mgr = RedisManager::GetInstance();
    redis_mgr -> del(REDIS_MESSAGEID_PREFIX + to_string(doc["message_id"].GetInt()));
    rsp_mgr -> set_content(get_json(ret, "成功", ""));
    return SUCCESS;
}

//通过互斥锁解决缓存击穿问题
int MessageHandler::GetRedisMessageByMutex(int user_id, int message_id)
{
    RedisManager *redis_mgr = RedisManager::GetInstance();
    int ret;
    string ret_json;
    //尝试从Redis获取缓存
    string redis_getmess_key = REDIS_MESSAGEID_PREFIX + to_string(message_id);
    string redis_getmess_lock = REDIS_MESSAGE_LOCK_PREFIX + to_string(message_id);
    ret_json = redis_mgr -> get(redis_getmess_key, ret);
    //若获取失败，使用互斥锁方案
    if(ret == REDIS_REPLY_NIL)
    {
        while(true)
        {
            int ret;
            ret = redis_mgr -> setnx(REDIS_MESSAGE_LOCK_PREFIX + to_string(message_id));
            log_mgr -> println("setnx ret =[%d]", ret);
            if(ret != SUCCESS)
            {
                usleep(1000);
                continue;
            }
            MessageInfo mess_info;
            ret = GetMessageFromDb(user_id, message_id, mess_info);
            if(ret != SUCCESS)
            {
                redis_mgr -> del(redis_getmess_lock);
                return ret;
            }
            JsonHelper::ObjectToJson(mess_info, ret_json);
            redis_mgr -> set(REDIS_MESSAGEID_PREFIX + to_string(message_id), ret_json);
            redis_mgr -> expire(REDIS_MESSAGEID_PREFIX + to_string(message_id), REDIS_MESSAGE_EXPIRETIME);
            redis_mgr -> del(redis_getmess_lock);
            break;
        }
        
    }
    rsp_mgr -> set_content(get_json(SUCCESS, "成功", string(ret_json)));
    return SUCCESS;
}

//通过设置逻辑过期解决缓存击穿问题
int MessageHandler::GetRedisMessageBylogicExpire(int user_id, int message_id)
{

}

//从数据库中获取消息
int MessageHandler::GetMessageFromDb(int user_id, int message_id, 
                                     MessageInfo &ret_message_info)
{
    int ret;
    RedisManager *redis_mgr = RedisManager::GetInstance();
    ssp::ReqInfoBase req_base;
    ssp::RspInfoBase rsp_base;
    char send_buf[10240], recv_buf[10240];
    req_base.set_mess_type(GET_MESSAGE_REQ);
    ssp::GetMessageReq *get_mess_req = req_base.mutable_get_mess_req();
    get_mess_req -> set_user_id(user_id);
    get_mess_req -> set_message_id(message_id);
    req_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage("message_write", send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "获取消息失败", ""));
        log_mgr -> println("消息服务器SendMessage失败");
        return ret;
    }
    ret = bus_mgr -> TryGetMessage("message_read", recv_buf, 10239, 10000, 500);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "获取消息失败", ""));
        log_mgr -> println("消息服务器TryGetMessage失败");
        return ret;
    }
    rsp_base.ParseFromArray(recv_buf, 10239);
    ssp::GetMessageRsp *get_mess_rsp = rsp_base.mutable_get_mess_rsp();
    ret = get_mess_rsp -> ret(); 
    log_mgr -> println("rsp_base.mess_type = %d", rsp_base.mess_type());
    log_mgr -> println("返回的message_id:%d", get_mess_rsp -> mutable_message_info() -> message_id());
    if (ret != SUCCESS || rsp_base.mess_type() != GET_MESSAGE_RSP)
    {
        //数据库获取消息失败，回种空值
        if(ret == DB_QUERY_FAIL)
        {
            redis_mgr -> set(REDIS_MESSAGEID_PREFIX + to_string(message_id), "");
            redis_mgr -> expire(REDIS_MESSAGEID_PREFIX + to_string(message_id), REDIS_MESSAGE_EXPIRETIME);
        }
        rsp_mgr -> set_content(get_json(ret, "获取消息失败", ""));
        log_mgr -> println("消息服务器返回了一个错误码：%d", ret);
        return ret;
    }
    ret_message_info.FromPb(get_mess_rsp -> message_info());
    log_mgr -> println("FromDb return message_id = [%d]", ret_message_info.message_id());
    return SUCCESS;
}
