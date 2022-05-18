/*************************************************************************
	> File Name: UserInfo.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Mon 10 Jan 2022 04:25:46 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"UserInfo.h"
using namespace ssp;
void UserInfo::FromPb(ssp::UserInfoBase &pb_user)
{
    set_user_name(pb_user.user_name());
    set_nick_name(pb_user.nick_name());
    set_reg_time(pb_user.reg_time());
    set_reg_from(pb_user.from());
    set_login_time(pb_user.login_time());
    set_last_login_time(pb_user.last_login_time());
    set_fresh_time(pb_user.fresh_time());
    set_password(pb_user.password());
    set_db_flag(0);
}
void UserInfo::ToPb(ssp::UserInfoBase &pb_user)
{ 
    pb_user.set_user_id(user_id());
    pb_user.set_user_name(user_name());
    pb_user.set_nick_name(nick_name());
    pb_user.set_reg_time(reg_time());
    pb_user.set_from(reg_from());
    pb_user.set_login_time(login_time());
    pb_user.set_last_login_time(last_login_time());
    pb_user.set_fresh_time(fresh_time());
    pb_user.set_password(password());
}

UserInfo::UserInfo()
{
    user_id_ = 0;
    user_name_ = "";
    nick_name_ = "";
    reg_time_ = 0;
    reg_from_ = 0;
    login_time_ = 0;
    logout_time_ = 0;
    last_login_time_ = 0;
    fresh_time_ = 0;
    password_ = "";
    db_flag_ = 0;
}

