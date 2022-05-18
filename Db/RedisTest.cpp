/*************************************************************************
	> File Name: RedisTest.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 17 Apr 2022 07:25:22 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"RedisManager.h"
using namespace std;
int main()
{
    RedisManager *redis = RedisManager::GetInstance();
    int status;
    redis -> set("123", "qaq");
    printf("set reply_type = [%d], reply_str = [%s]\n", 
           redis -> _reply -> type, redis -> _reply -> str);
    redis -> get("123", status);
    printf("get reply_type = [%d], reply_str = [%s]\n", 
           redis -> _reply -> type, redis -> _reply -> str);
    redis -> expire("123", "10");
    printf("expire reply_type = [%d], reply_str = [%s]\n", 
           redis -> _reply -> type, redis -> _reply -> str);
    redis -> setnx("123");
    printf("setnx reply_type = [%d], reply_str = [%s]\n", 
           redis -> _reply -> type, redis -> _reply -> str);
    redis -> del("123");
    printf("del reply_type = [%d], reply_str = [%s]\n", 
           redis -> _reply -> type, redis -> _reply -> str);
    return 0;


}
