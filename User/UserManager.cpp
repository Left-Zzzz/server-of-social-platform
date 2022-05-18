/*************************************************************************
	> File Name: UserManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 12:08:11 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"UserManager.h"
#include"UserInfo.h"
#include"LogManager.h"
#include<sys/file.h>
using namespace std;

DbManager db_svr;

int UserManager::Init()
{
    //指定日志路径
    const char *file_path = "user_svr.log";
    //初始化日志
    log_mgr -> log_fp_init(file_path);
    //初始化Db
    int ret = db_svr.initDb();
    log_mgr -> println("User Manager Init.\n");
    return ret;
}

void UserManager::Start()
{
    int ret = Init();
    if(ret != SUCCESS)
    {
        log_mgr -> println("UserManager Start Fail with code %d\n", ret);
        
        return;
    }
    log_mgr -> println("User Manager Start.\n");
    
}

int UserManager::Shutdown()
{
    log_mgr -> println("User Manager Shutdown.\n");
    
}

void UserManager::Proc()
{
    log_mgr -> println("User Manager Proc.\n");
    
}

void UserManager::Restart()
{
    log_mgr -> println("User Manager Restart.\n");
    
}

//根据userid获取UserInfo
int UserManager::GetUser(const int user_id, UserInfo *user)
{
    if(!user) return -12345;
    int ret = db_svr.execSql("select * from tb_user where user_id = " + to_string(user_id) + ";");
    if(ret != SUCCESS) return ret;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(!db_svr.row) return USER_NOT_EXIST;
    //1.user_id 2.user_name 3.password 4.nick_name 5.reg_time 6.reg_from 7.login_time 8.logout_time 9.fresh_time
    user -> set_user_id(atoi(db_svr.row[0]));
    user -> set_user_name(db_svr.row[1]);
    user -> set_password(db_svr.row[2]);
    user -> set_nick_name(db_svr.row[3]);
    user -> set_reg_time(atoi(db_svr.row[4]));
    user -> set_reg_from(atoi(db_svr.row[5]));
    user -> set_login_time(atoi(db_svr.row[6]));
    user -> set_logout_time(atoi(db_svr.row[7]));
    user -> set_fresh_time(atoi(db_svr.row[8]));
    ssp::UserInfoBase pb_user;
    log_mgr -> println("getuser: id:[%d],user_name:[%s]\n", user -> user_id(), user -> user_name().c_str());
    
    db_svr.freeRes();
    
    return SUCCESS;
}

int UserManager::GetUser(const char *user_name, UserInfo *user)
{
    string user_name_str = user_name;
    return GetUser(user_name_str, user);
}

//根据username获取UserInfo
int UserManager::GetUser(const string &user_name, UserInfo *user)
{
    string Sql = "select * from tb_user where user_name = \'" + user_name + "\';";
    log_mgr -> println("Sql语句:%s\n", Sql.c_str());
    
    int ret = db_svr.execSql(Sql);
    if(ret != SUCCESS) return ret;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(!db_svr.row) return USER_NOT_EXIST;
    log_mgr -> println("db_query_ret = %d\n", ret);
    
    //unsigned long *lengths = mysql_fetch_lengths(db_svr.result);
    //1.user_id 2.user_name 3.password 4.nick_name 5.reg_time 6.reg_from 7.login_time 8.logout_time 9.fresh_time
    if(db_svr.row[0]) user -> set_user_id(atoi(db_svr.row[0]));
    if(db_svr.row[1]) user -> set_user_name(db_svr.row[1]);
    if(db_svr.row[2]) user -> set_password(db_svr.row[2]);
    if(db_svr.row[3]) user -> set_nick_name(db_svr.row[3]);
    if(db_svr.row[4]) user -> set_reg_time(atoi(db_svr.row[4]));
    if(db_svr.row[5]) user -> set_reg_from(atoi(db_svr.row[5]));
    if(db_svr.row[6]) user -> set_login_time(atoi(db_svr.row[6]));
    if(db_svr.row[7]) user -> set_logout_time(atoi(db_svr.row[7]));
    if(db_svr.row[8]) user -> set_fresh_time(atoi(db_svr.row[8]));
    //ssp::UserInfoBase pb_user;
    //pb_user.ParseFromArray(db_svr.row[1], lengths[1]);
    log_mgr -> println("getuser:id:[%d],user_name:[%s]\n", user -> user_id(), user -> user_name().c_str());
    
    db_svr.freeRes();
    
    return SUCCESS;
}

//获取全局UserId
int UserManager::GetUserGlbId()
{
    string createSql = "update tb_var set value = value + 1 where str = \'user_id\'";
    int ret;
    if((ret = db_svr.execSql(createSql)) != SUCCESS)
        return ret;

    createSql = "select value from tb_var where str = \'user_id\'";
    if((ret = db_svr.execSql(createSql)) != SUCCESS)
        return ret;

    db_svr.row = mysql_fetch_row(db_svr.result);
    if(db_svr.row[0] == NULL)
    {
        log_mgr -> println("%s\n", mysql_error(db_svr.conn));
        
        return DB_QUERY_FAIL;
    }
    ret = atoi(db_svr.row[0]);
    mysql_free_result(db_svr.result);
    log_mgr -> println("GetUserGlbId:ret = [%d]\n", ret);
    
    return ret;
}

int UserManager::CreateUser(const string &user_name, const string &pswd, int from)
{
    return UserManager::CreateUser(user_name.c_str(), pswd.c_str(), from);
}

int UserManager::CreateUser(const char *user_name, const char *pswd, int from)
{
    int ret, user_id;
    log_mgr -> println("reg:user_name = [%s], pswd = [%s]\n", user_name, pswd);
    
    UserInfo user;
    //找不到该用户
    ret = CheckExist(user_name);
    if(ret != USER_NOT_EXIST) return ret;

    log_mgr -> println("reg:USER_NOT_EXIST");
    
    if((user_id = GetUserGlbId()) < 0) return DB_QUERY_FAIL;
    user.set_user_id(user_id);
    user.set_user_name(user_name);
    user.set_password(pswd);
    //user.set_db_flag(USER_FLAG_CREATE);
    user.set_nick_name(user_name);
    user.set_reg_time(time(0));
    user.set_fresh_time(0);
    user.set_reg_from(from);
    log_mgr -> println("id_from_db = %d.\n", user.user_id());
    
    string Sql = "insert into tb_user values(" + to_string(user.user_id()) + ", '" + user.user_name() +"', '" + user.password() + "','" + user.nick_name() + "', " + to_string(user.reg_time()) + ", " + to_string(user.reg_from()) + ", " + to_string(user. login_time()) + ", " + to_string(user.logout_time()) + ", " + to_string(user.fresh_time()) + ");";
    ret = db_svr.execSql(Sql);

    //todo:共享内存实现创建关系表
    //CreateRelationRequest(user.user_id());
    return ret;
}

int UserManager::DeleteUser(int user_id)
{
    //重写逻辑。从数据库里删除。
    for(int i = 0; i < user_count_; i++)
    {
        if(users[i].user_id() == user_id)
        {
            //log_mgr -> println("USER EXIST %d %s.\n", users[i].user_id(), users[i].user_name().c_str());
            //users[i].set_db_flag(USER_FLAG_DELETE);
            return SUCCESS;
        }
    }
    
    return USER_NOT_EXIST;
}


int UserManager::CheckExist(const string &user_name)
{
    int ret, flag = 0;
    ret = db_svr.execSql("select * from tb_user where user_name = \'" + user_name + "';");
    if(ret != SUCCESS) return ret;
    if(db_svr.result && mysql_fetch_row(db_svr.result)) 
        flag = 1;
    string logmsg = "CheckExist: existflag = [" + to_string(flag) + "]";
    log_mgr -> println(logmsg.c_str());
    
    db_svr.freeRes();
    if(!flag) return USER_NOT_EXIST;
    return USER_EXIST;
}

int UserManager::CheckExist(const int user_id)
{
    int ret, flag = 0;
    ret = db_svr.execSql("select * from tb_user where user_id = " + to_string(user_id) + ";");
    if(ret != SUCCESS) return ret;
    if(db_svr.result && mysql_fetch_row(db_svr.result)) 
    {
        db_svr.freeRes();
        flag = 1;
    }
    string logmsg = "CheckExist: existflag = [" + to_string(flag) + "]";
    log_mgr -> println(logmsg.c_str());
    
    if(!flag) return USER_NOT_EXIST;
    return USER_EXIST;
}

int UserManager::LoginCheck(const std::string &user_name, const std::string &user_pswd)
{
    log_mgr -> println("LoginCheck\n");
    
    UserInfo user;
    int ret = GetUser(user_name, &user);
    if(ret != SUCCESS) return ret;

    if(user.password() != user_pswd)
        return WRONG_PASSWORD;

    return SUCCESS;
}

int UserManager::LoginCheck(char *user_name, char *user_pswd)
{
    return LoginCheck(user_name, user_pswd);
}

int UserManager::GetUserIdByUserName(const string &user_name)
{
    int ret;
    string Sql = "select user_id from tb_user where user_name = \'" + user_name + "\';";
    log_mgr -> println("GetUserIdByUserName");
    string logmsg = "Sql:" + Sql;
    log_mgr -> println(logmsg.c_str());
    
    ret = db_svr.execSql(Sql);
    if(ret != SUCCESS) return ret;
    if(!db_svr.result) return USER_NOT_EXIST;
    db_svr.row = mysql_fetch_row(db_svr.result);
    if(!db_svr.row) return USER_NOT_EXIST;
    int user_id = atoi(db_svr.row[0]);
    log_mgr -> println("user_id from db:[%d]", user_id);
    db_svr.freeRes();
    return user_id;
}

//登出
int UserManager::Logout(int user_id, int now)
{
    string Sql = "update tb_user set logout_time = " + to_string(now) + " where user_id = " + to_string(user_id) + ";";
    int ret = db_svr.execSql(Sql);
    return ret;
}

//更新登录时间
int UserManager::UpdateUserLoginTime(int user_id,int time_now)
{
    string Sql = "update tb_user set login_time = " + to_string(time_now) + " where user_id = " + to_string(user_id) + ";";
    int ret = db_svr.execSql(Sql);
    return ret;
}

//更改刷新时间
int UserManager::UpdateFreshTime(int user_id, int now)
{
    string Sql = "update tb_user set fresh_time = " + to_string(now) + " where user_id = " + to_string(user_id) + ";";
    int ret = db_svr.execSql(Sql);
    return ret;
}

UserManager::UserManager()
{
    log_mgr = LogManager::GetInstance();
    Start();
}

UserManager::~UserManager()
{
    delete log_mgr;
    Shutdown();
}

