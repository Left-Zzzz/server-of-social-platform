/*************************************************************************
	> File Name: RelationHandler.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 20 Mar 2022 02:51:17 PM CST
 ************************************************************************/

#include"Handler/RelationHandler.h"

RelationHandler *RelationHandler::instance_ = new RelationHandler();

RelationHandler *RelationHandler::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new RelationHandler();
    return instance_;
}

RelationHandler::RelationHandler()
{
    //往MainHandler的handler_list中注册script
    handler_list[SCRIPT_NAME_] = this;
    auth_script_list.insert(SCRIPT_NAME_);
    if(handler_list.count(SCRIPT_NAME_))
        log_mgr -> println("[RelationHandler]:注册成功！");
    else log_mgr -> println("[RelationHandler]:注册失败！");
}

RelationHandler::~RelationHandler()
{
    //往MainHandler的handler_list中删除script
    auto it = MainHandler::handler_list.find(SCRIPT_NAME_);
    if(it != MainHandler::handler_list.end())
        MainHandler::handler_list.erase(it);
}

//处理请求
int RelationHandler::Handle(string &req_method, int user_id, string &input_data)
{
    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
    return REQ_METHOD_NOT_EXIST;

}

//处理请求
int RelationHandler::Handle(string &req_method, int user_id, stringkv &input_data)
{
    log_mgr -> println("[RelationHandler]:user_id=[%d]", user_id);
    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
    return REQ_METHOD_NOT_EXIST;
}

int RelationHandler::Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query)
{
    log_mgr -> println("[RelationHandler]:user_id=[%d]", user_id);
    if(input_data_query.count("addfriend"))
    {
        return AddFriendHandle(req_method, user_id, input_data_body, input_data_query);
    }

    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
    return REQ_METHOD_NOT_EXIST;
}

int RelationHandler::AddFriendHandle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query)
{
    ssp::ReqInfoBase req_base;
    ssp::RspInfoBase rsp_base;
    char send_buf[10240], recv_buf[10240];
    Document doc;
    doc.Parse<kParseStopWhenDoneFlag>(input_data_body.c_str());
    int ret;
    if(!doc.IsObject() || !doc.HasMember("friend_id") || !doc["friend_id"].IsInt())
    {
        rsp_mgr -> set_content(get_json(QUERY_MESSAGE_ERR, "请求的信息错误", ""));
        rsp_mgr -> set_status(to_string(400));
        return QUERY_MESSAGE_ERR;
    }
    ssp::AddFriendReq *pbreq = req_base.mutable_add_friend_req();
    req_base.set_mess_type(ADD_FRIEND_REQ);
    pbreq -> set_user_id(user_id);
    pbreq -> set_other_id(doc["friend_id"].GetInt());
    log_mgr -> println("mess_type : %d\n", req_base.mess_type());
    req_base.SerializeToArray(send_buf, 10239);
    log_mgr -> println("ret_size = [%d]\n", req_base.ByteSizeLong());
    ret = bus_mgr -> SendMessage(string("relaiton_write"), send_buf, req_base.ByteSizeLong());
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "服务器响应超时！", ""));
        return ret;
    }
    ret = bus_mgr -> TryGetMessage(string("relation_read"), recv_buf, 10239, 500, 20);
    if(ret != SUCCESS)
    {
        rsp_mgr -> set_content(get_json(ret, "服务器响应超时！", ""));
        return ret;
    }
    rsp_base.ParseFromArray(recv_buf, 10239);
    ssp::AddFriendRsp *pbrsp = rsp_base.mutable_add_friend_rsp();
    ret = pbrsp -> ret();
    log_mgr -> println("pbrsp -> ret = [%d]", ret);
    if(ret == SUCCESS)
        rsp_mgr -> set_content(get_json(ret, "添加好友成功！", ""));
    else 
        rsp_mgr -> set_content(get_json(ret, "添加好友失败！", ""));
    return ret;
}
