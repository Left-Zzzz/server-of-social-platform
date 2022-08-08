/*************************************************************************
	> File Name: CookieManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 19 May 2022 11:53:17 PM CST
 ************************************************************************/

#ifndef _COOKIEMANAGER_H
#define _COOKIEMANAGER_H
#include"CookieInfo.h"
#include"common/ret_value.h"
#include"LogManager.h"
#include<unordered_map>
#include<string>
#include<vector>
using namespace std;
class CookieManager
{
public:
	//TODO 代办 分割cookie到CookieInfo，存入哈希表中。
	int SplitCookies(char *cookies_raw);
};
#endif
