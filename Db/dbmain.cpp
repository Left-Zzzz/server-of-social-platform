/*************************************************************************
	> File Name: dbmain.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 05:36:58 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"DbManager.h"
using namespace std;
int main()
{
    DbManager db_svr;
    int ret = db_svr.initDb("127.0.0.1", "root", "left1234", "ssp");
    printf("ret = %d\n", ret);
    db_svr.execSql("select * from dbtest");
    db_svr.execSql("insert into dbtest values(2, 'EFC', 134, 'adsfhsdahf');");
    db_svr.execSql("select * from dbtest;");
    db_svr.execSql("update dbtest set str = 'MNK' where id = 2;");
    db_svr.execSql("select * from dbtest;");
    db_svr.execSql("delete from dbtest where id = 2");
    return 0;
}
