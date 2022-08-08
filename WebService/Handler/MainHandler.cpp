/*************************************************************************
	> File Name: MainHandler.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Fri 18 Mar 2022 03:29:49 PM CST
 ************************************************************************/

#include"Handler/MainHandler.h"
using namespace rapidjson;
//单例，类外初始化
MainHandler *MainHandler::instance_ = NULL;
LogManager *MainHandler::log_mgr = LogManager::GetInstance();
UserManager MainHandler::user_mgr;
TokenManager *MainHandler::token_mgr = TokenManager::GetInstance();
unordered_set<string> MainHandler::auth_script_list;
unordered_map<string, MainHandler*> MainHandler::handler_list;
ResponseManager *MainHandler::rsp_mgr = ResponseManager::GetInstance();
BusManager *MainHandler::bus_mgr = BusManager::GetInstance();

string MainHandler::get_json(int code, const char *message, string &&data)
{
    Document doc;
    Document data_val;
    Document::AllocatorType &alloc_doc = doc.GetAllocator();
    Document::AllocatorType &alloc_data = doc.GetAllocator();
    doc.SetObject();
    doc.AddMember("code", code, alloc_doc);
    Value mess_val;
    mess_val = StringRef(message);
    doc.AddMember("message", mess_val, alloc_doc);
    if(data != "")
    {
        data_val.Parse<kParseStopWhenDoneFlag>(data.c_str());
        doc.AddMember("data", data_val, alloc_data);
    }
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

int MainHandler::CommonHandle(stringkv &cookies_kv, string &uri, int &user_id)
{
    //获取rsp_mgr单例，防止野指针导致程序崩溃
    rsp_mgr = ResponseManager::GetInstance();
    //调用AuthToken
    TokenManager *token_mgr = TokenManager::GetInstance();
    //如果cookie存在，调用ValidateToken进行验证
    if(cookies_kv.count("ssp_login"))
    {
        log_mgr -> println("收到cookie：ssp_login, val = [%s]", cookies_kv["ssp_login"].c_str());
        user_id = token_mgr -> ValidateToken(cookies_kv["ssp_login"]);
    }
    //否则将user_id设置为用户不存在
    else user_id = USER_NOT_EXIST;
    log_mgr -> println("user_id = [%d]", user_id);
    if(auth_script_list.count(uri))
    {
        if(user_id < 0)
        {
            rsp_mgr -> set_status("401 Unauthorized");
            //返回需要用户验证的JSON消息
            return USER_UNAUTH;
        }
    }
    //调用对应handler
    //如果没有对应handler，返回非法script
    if(!handler_list.count(uri))
    {
        //返回非法script
        rsp_mgr -> set_status("404 Not Found");
        return SCRIPT_NOT_EXIST;
    }
    return SUCCESS;
}

MainHandler *MainHandler::GetInstance()
{
    if(instance_ != NULL) return instance_;
    instance_ = new MainHandler();
    return instance_;
}
/*
 * input_data类型：POST下Json为string类型，其他为unorder_map<string, sting>,即stringkv类型
 **/
int MainHandler::Handle(string &req_method, stringkv &cookies_kv, string &uri, string &input_data)
{
    int ret, user_id;
    if((ret = CommonHandle(cookies_kv, uri, user_id)) != SUCCESS)
        return ret;
    handler_list[uri] -> Handle(req_method, user_id, input_data);
    return SUCCESS;
}

int MainHandler::Handle(string &req_method, stringkv &cookies_kv, string &uri, stringkv &input_data)
{
    int ret, user_id;
    if((ret = CommonHandle(cookies_kv, uri, user_id)) != SUCCESS)
        return ret;
    handler_list[uri] -> Handle(req_method, user_id, input_data);
    return SUCCESS;
}

/*
 * json可以附带参数的Handle方法
 * */

int MainHandler::Handle(string &req_method, stringkv &cookies_kv, string &uri, string &input_data_json, stringkv &input_data_query)
{
    int ret, user_id;
    if((ret = CommonHandle(cookies_kv, uri, user_id)) != SUCCESS)
        return ret;
    handler_list[uri] -> Handle(req_method, user_id, input_data_json, input_data_query);
    return SUCCESS;
}

MainHandler::MainHandler()
{
    //往MainHandler的handler_list中注册script
    handler_list[SCRIPT_NAME_] = this;
    log_mgr = LogManager::GetInstance();
}

MainHandler::~MainHandler()
{
    //往MainHandler的handler_list中删除script
    auto it = handler_list.find(SCRIPT_NAME_);
    if(it != handler_list.end())
        handler_list.erase(it);
}

int MainHandler::Handle(string &req_method, int user_id, string &input_data)
{
    rsp_mgr -> set_status("404 Not Found");
    return SCRIPT_NOT_EXIST;
}

int MainHandler::Handle(string &req_method, int user_id, stringkv &input_data)
{
    rsp_mgr -> set_status("404 Not Found");
    return SCRIPT_NOT_EXIST;
}

int MainHandler::Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query)
{
    return Handle(req_method, user_id, input_data_body);
}
