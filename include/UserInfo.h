/*************************************************************************
	> File Name: UserInfo.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 08 Jan 2022 11:12:48 PM CST
 ************************************************************************/

#ifndef _USERINFO_H
#define _USERINFO_H
#include<string.h>
#include"common/macro.h"
#include"proto/user_info_base.pb.h"
struct UserInfo
{
public:
    void FromPb(ssp::UserInfoBase &pb_user);
    void ToPb(ssp::UserInfoBase &pb_user);
    UserInfo();
private:
    //1.user_id 2.user_name 3.password 4.nick_name 5.reg_time 6.reg_from 7.login_time 8.logout_time 9.fresh_time
    GETSETVAR(int, user_id)
    GETSETVAR(std::string, user_name)
    GETSETVAR(std::string, nick_name)
    GETSETVAR(int, reg_time)
    GETSETVAR(int, reg_from)
    GETSETVAR(int, login_time)
    GETSETVAR(int, logout_time)
    GETSETVAR(int, last_login_time)
    GETSETVAR(int, fresh_time)
    GETSETVAR(std::string, password)
    GETSETVAR(int, db_flag)
};
#endif
