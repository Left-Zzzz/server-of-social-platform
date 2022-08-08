/*************************************************************************
	> File Name: MainHandler.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 19 Mar 2022 03:29:14 PM CST
 ************************************************************************/

#ifndef _MAINHANDLER_H
#define _MAINHANDLER_H

#include<bits/stdc++.h>
#include"ResponseManager.h"
#include"common/ret_value.h"
#include"common/messsage_type.h"
#include"Auth/AuthLib.h"
#include"RedisManager.h"
#include"LogManager.h"
#include"UserManager.h"
#include"BusManager.h"
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
#include<rapidjson/writer.h>
#include"proto/message_define.pb.h"
using namespace std;
using namespace rapidjson;
using stringkv=unordered_map<string, string>;
class MainHandler
{
private:
    const string SCRIPT_NAME_ = "/";
    static MainHandler *instance_;
    int CommonHandle(stringkv &cookies_kv, string &uri, int &user_id);
    MainHandler(const MainHandler&) = delete;
    MainHandler &operator=(const MainHandler&) = delete;
protected:
    MainHandler();
    virtual ~MainHandler();
    //需要验证的script列表
    static unordered_set<string> auth_script_list;
    //调用的对应handler的集合
    static unordered_map<string, MainHandler*> handler_list;
    string get_json(int code, const char *message, string &&data);
    static LogManager *log_mgr;
    static UserManager user_mgr;
    static TokenManager *token_mgr;
    static ResponseManager *rsp_mgr;
    static BusManager *bus_mgr;
public:
    static MainHandler* GetInstance();
    //input_data:POST下type为JOSN则为string类型
    int Handle(string &req_method, stringkv &cookies_kv, string &uri, string &input_data);
    int Handle(string &req_method, stringkv &cookies_kv, string &uri, string &input_data_json, stringkv &input_data_query);
    //GET下为stringkv类型
    int Handle(string &req_method, stringkv &cookies_kv, string &uri, stringkv &input_data);
    virtual int Handle(string &req_method, int user_id, string &input_data);
    virtual int Handle(string &req_method, int user_id, stringkv &input_data);
    virtual int Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query);
};

#endif
