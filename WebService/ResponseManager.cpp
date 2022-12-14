/*************************************************************************
	> File Name: ResponseManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 29 Jan 2022 04:40:32 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"ResponseManager.h"
#include"TimeCommon.h"
#include"common/ret_value.h"
using namespace std;

//单例：类外初始化
ResponseManager *ResponseManager::instance_ = NULL;

ResponseManager *ResponseManager::GetInstance()
{
    if(instance_ != NULL) return instance_;
    instance_ = new ResponseManager();
    return instance_;
}

string ResponseManager::GetResponse()
{
    string retdata;
    //打印回复头
    if(status_ != "")
    {
        retdata += "Status:" + status_ + "\r\n";
    }
    if(content_type_ != "")
    {
        retdata += "Content-type: "+ content_type_ + "\r\n";
    }
    else
    {
        //默认内容类型
        retdata += "Content-type: text/html;charset=utf-8\r\n";
    }
    retdata += "Content-Length: " + to_string(content_.size()) + "\r\n";
    if(!cookies_.empty())
    {
        for(const auto &s : cookies_)
            retdata += "Set-Cookie: " + s + "\r\n";
    }
    retdata += "\r\n";
    retdata += content_;
    //printf("\r\n");
    return retdata;
}

int ResponseManager::set_cookie(std::string name, std::string &value,\
     time_t expire, int max_age, std::string &&path)
{
    /* 设置cookie，格式按浏览器规范格式输出
     *
     */
    //Set-Cookie: userName=admin; Max-Age=3600; Path=/; Expires=Tue, 17 May 2022 17:03:34 GMT
    std::string expire_str = TimeCommon::ConvertTimestamp(
        expire, "%a, %d %b %Y %H:%M:%S GMT");
    std::string cookie_info = name + "=" + value + "; Max-Age=" + 
    to_string(max_age) + "; Path=" + path + "; Expires=" + expire_str;
    cookies_.push_back(std::move(cookie_info));
    return SUCCESS;
}

ResponseManager::ResponseManager()
{
    status_="";
    content_type_="";
    content_="";
}

ResponseManager::~ResponseManager()
{
    instance_ = NULL;
}

