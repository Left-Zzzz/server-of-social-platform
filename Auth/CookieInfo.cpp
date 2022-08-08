/*************************************************************************
	> File Name: CookieInfo.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Fri 20 May 2022 08:38:51 PM CST
 ************************************************************************/

#include"Auth/CookieInfo.h"
using namespace std;
LogManager *CookieInfo::log_mgr = LogManager::GetInstance();
unordered_map<string, string> CookieInfo::cookies_map;

int CookieInfo::SingleCookieStorage(char *cookie_raw)
{
	/*
	 * 定义从HTTP拿到的单条cookie数据储存方法
	 * param: char *cookie_raw, 从http请求中获取的单条cookie原始数据
	 * param: CookieInfo &cookie_info, 返回的cookie_info信息
	 * return: 状态码, 成功返回SUCCESS, 失败返回对应错误码 
	 */
	string key = strtok(cookie_raw, "=");
	if(key == "") return COOKIE_KEY_NONE;
	string val = strtok(NULL, "\0");
	if(val == "") return COOKIE_VAL_NONE;
	log_mgr -> println("SingleCookieStorage: key = [%s], val = [%s]", key, val);
	cookies_map[key] = val;
	return SUCCESS;
}
