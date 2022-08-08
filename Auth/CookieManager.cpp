/*************************************************************************
	> File Name: CookieManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 19 May 2022 11:52:17 PM CST
 ************************************************************************/

#include"Auth/CookieManager.h"
int CookieManager::SplitCookies(char *cookies_raw)
{
    /* 
     * 将http请求拿到的原始cookie数据切分成每一条cookie原始数据
     * param:char *cookies_raw, 从http请求中拿到的原始cookies数据
     * return: 成功返回SUCCESS, 失败返回对应错误码
     */
    LogManager *log_mgr = LogManager::GetInstance();
    log_mgr -> println("cookies: [%s]",cookies_raw);
    vector<char *> cookie_list;
    char *cookie_raw;
    cookie_raw = strtok(cookies_raw, ";");
    if(!cookie_raw) return COOKIE_EMPTY;
    cookie_list.push_back(cookie_raw);
    while(cookie_raw = strtok(NULL, ";"))
        cookie_list.push_back(cookie_raw);
    for(char *p_cookie : cookie_list)
    {
        CookieInfo info;
        int ret = CookieInfo::SingleCookieStorage(p_cookie);
        if(ret != SUCCESS)
        {
            log_mgr -> println("SingleCookieStorage fail. p_cookie=[%s], ret=[%d]",
                                p_cookie, ret);
        }
    }
    return SUCCESS;
}