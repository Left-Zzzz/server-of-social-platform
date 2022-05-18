/*************************************************************************
	> File Name: DbManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 04:46:36 PM CST
 ************************************************************************/

#ifndef _DBMANAGER_H
#define _DBMANAGER_H
#include<bits/stdc++.h>
#include"mysql/mysql.h"
#include"common/ret_value.h"
#include"common/macro.h"
#include"proto/user_info_base.pb.h"
#include"UserInfo.h"
#include"LogManager.h"

using namespace std;

class DbManager
{
    GETSETSTR(64, host)
    GETSETSTR(64, user)
    GETSETSTR(64, pswd)
    GETSETSTR(64, db_name)
public:
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
public:
    DbManager();
    int fetch_row();
    int initDb();
    int initDb(string host, string user, string pswd, string db_name);
    int execSql(const string &sql);
    int freeRes();
private:
    LogManager *log_mgr_;
    GETSETVAR(int, num_rows);
    GETSETVAR(int, num_cols);
    int connectDb();
    int ReadDbConfig();
};
#endif
