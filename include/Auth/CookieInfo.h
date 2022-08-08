/*************************************************************************
	> File Name: CookieInfo.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 19 May 2022 11:54:34 PM CST
 ************************************************************************/

#ifndef _COOKIEINFO_H
#define _COOKIEINFO_H
#include"common/ret_value.h"
#include"LogManager.h"
#include<string.h>
#include<string>
#include<unordered_map>
using namespace std;

class CookieInfo
{
private:
	static LogManager *log_mgr;
public:
	//储存cookie信息的容器
	static unordered_map<string, string> cookies_map;
public:
	//定义从HTTP拿到的单条cookie数据储存方法
	static int SingleCookieStorage(char *cookie_raw);
};
#endif
