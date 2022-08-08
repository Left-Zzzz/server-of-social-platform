/*************************************************************************
	> File Name: LoginHandler.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 20 Mar 2022 03:29:14 PM CST
 ************************************************************************/

#ifndef _LOGINHANDLER_H
#define _LOGINHANDLER_H

#include"MainHandler.h"

class LoginHandler:virtual public MainHandler
{
private:
    LoginHandler();
    ~LoginHandler();
    LoginHandler(LoginHandler&);
    LoginHandler operator=(LoginHandler&);
    static LoginHandler *instance_;
    const string SCRIPT_NAME_ = "/user/login";
public:
    virtual int Handle(string &req_method, int user_id, string &input_data);
    virtual int Handle(string &req_method, int user_id, stringkv &input_data);
    static LoginHandler *GetInstance();
    int Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query);
    int HandleLogin(string &req_method, int user_id, stringkv &input_data);

};
#endif
