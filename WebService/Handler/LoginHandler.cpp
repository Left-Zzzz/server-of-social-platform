/*************************************************************************
	> File Name: LoginHandler.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 20 Mar 2022 02:51:17 PM CST
 ************************************************************************/

#include"Handler/LoginHandler.h"

LoginHandler *LoginHandler::instance_ = new LoginHandler();

LoginHandler *LoginHandler::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new LoginHandler();
    return instance_;
}

LoginHandler::LoginHandler()
{
    //往MainHandler的handler_list中注册script
    handler_list[SCRIPT_NAME_] = this;
    if(handler_list.count(SCRIPT_NAME_))
        log_mgr -> println("[LoginHandler]:注册成功！");
    else log_mgr -> println("[LoginHandler]:注册失败！");

}

LoginHandler::~LoginHandler()
{
    //往MainHandler的handler_list中删除script
    auto it = MainHandler::handler_list.find(SCRIPT_NAME_);
    if(it != MainHandler::handler_list.end())
        MainHandler::handler_list.erase(it);
}

//处理请求
int LoginHandler::Handle(string &req_method, int user_id, string &input_data)
{
    ResponseManager *rsp_mgr = ResponseManager::GetInstance();
    rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));

}

//处理请求
int LoginHandler::Handle(string &req_method, int user_id, stringkv &input_data)
{
    log_mgr -> println("[LoginManager]:user_id=[%d]", user_id);
    ResponseManager *rsp_mgr = ResponseManager::GetInstance();
    if(req_method == "POST")
    {
        return HandleLogin(req_method, user_id, input_data);
    }
    else
    {
        rsp_mgr -> set_content(get_json(REQ_METHOD_NOT_EXIST, "请求方法不可用！", ""));
        return REQ_METHOD_NOT_EXIST;
    }
    
    return SUCCESS;
}

int LoginHandler::Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query)
{
    return Handle(req_method, user_id, input_data_body);
}

int LoginHandler::HandleLogin(string &req_method, int user_id, stringkv &input_data)
{
    if(user_id >= 0)
    {
        rsp_mgr -> set_content(get_json(USER_EXIST, "已登录，无需重复登录！", ""));
        return USER_EXIST;
    }
    //这里应该是查表单
    if(!input_data.count("user_name") || !input_data.count("password"))
    {
        rsp_mgr -> set_content(get_json(VALIDATE_FAILED, "请求的信息错误!", ""));
        return VALIDATE_FAILED;
    }
    //成功处理
    else
    {
        int ret = -1;
        log_mgr -> println("user_name = [%s], password = [%s]\n", input_data["user_name"].c_str(), input_data["password"].c_str());
        
        ret = user_mgr.LoginCheck(input_data["user_name"], input_data["password"]);
        if(ret != SUCCESS)
        {
            rsp_mgr -> set_content(get_json(ret, "登录失败", ""));
            return ret;
        }
        //调用UserSvr的Login，成功的话设置Cookie
        else
        {
            int user_id = user_mgr.GetUserIdByUserName(input_data["user_name"]);
            rsp_mgr -> set_content(get_json(0, "登录成功", ""));
            //设置cookie
            string user_id_str = to_string(user_id);
            string ret_token = token_mgr -> CreateToken(user_id_str);
            // set_cookie 交给TokenManager来做
            return token_mgr -> CreateTokenCookie(user_id_str);
        }
    }
}