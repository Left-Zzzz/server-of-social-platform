/*************************************************************************
    > File Name: UserManager.h
    > Author: Left
    > Mail: 1059051242@qq.com
    > Created Time: Sun 09 Jan 2022 12:04:27 PM CST
 ************************************************************************/

#ifndef _USERMANAGER_H
#define _USERMANAGER_H
#include"LogManager.h"
#include"DbManager.h"
#include"UserInfo.h"
#include"common/ret_value.h"
#include<unistd.h>
#include<bits/stdc++.h>
using namespace ssp;
class UserManager
{
private:
    LogManager *log_mgr;
    UserInfo users[10240];
    GETSETVAR(int, user_count);
public:
    int Init();
    void Start();
    void Proc();
    int Shutdown();
    void Restart();
    UserManager();
    ~UserManager();
public:
    void logfp_init(char *file_path);
    int GetUser(const int user_id, UserInfo *user);
    int GetUser(const char *user_name, UserInfo *user);
    int GetUser(const string &user_name, UserInfo *user);
    int CheckExist(const int user_id);
    int CheckExist(const string &user_name);
    int CreateUser(const char *user_name, const char *pswd, int from);
    int CreateUser(const string &user_name, const string &pswd, int from);
    int DeleteUser(int user_id);
    int LoginCheck(char *user_name, char *user_pswd);
    int LoginCheck(const string &user_name, const string &user_pswd);
    int GetUserIdByUserName(const string &user_name);
    int UpdateUserLoginTime(int user_id, int time_now);
    int GetUserGlbId();
    int Logout(int user_id, int now);
    int UpdateFreshTime(int user_id, int now);
};
#endif
