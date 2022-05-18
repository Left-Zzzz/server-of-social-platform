/*************************************************************************
	> File Name: DbManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 04:54:29 PM CST
 ************************************************************************/

#include"DbManager.h"
#define CONFIG_FILEPATH "db_config.ini"

DbManager::DbManager()
{
}

int DbManager::connectDb()
{
    conn = mysql_init(NULL);
    if(conn == NULL)
    {
        return DB_CONN_INIT_FAIL;
    }
    conn = mysql_real_connect(conn, host(), user(), pswd(), db_name(), 0, NULL, 0);
    if(conn == NULL)
    {
        return DB_CONN_CONNECT_FAIL;
    }
    my_bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_query(conn, "set names utf8");

    return SUCCESS;
}

int DbManager::initDb(string host, string user, string pswd, string db_name)
{
    set_host(host.c_str());
    set_user(user.c_str());
    set_pswd(pswd.c_str());
    set_db_name(db_name.c_str());
    return connectDb();
}

int DbManager::ReadDbConfig()
{
    FILE *fp = fopen(CONFIG_FILEPATH, "r");
    if(!fp) 
    {
        this -> log_mgr_ -> println("[DbManager]获取配置文件失败！\n");
        exit(1);
    }
    fscanf(fp, "host=%s\n", host_);
    fscanf(fp, "user=%s\n", user_);
    fscanf(fp, "pswd=%s\n", pswd_);
    fscanf(fp, "db_name=%s\n", db_name_);
    this -> log_mgr_ -> println("[DbManager]获取配置文件成功！[%s][%s][%s][%s]\n", host_, user_, pswd_, db_name_);
    fclose(fp);
}

//通过配置文件初始化DbMgr
int DbManager::initDb()
{
    log_mgr_ = LogManager::GetInstance();
    log_mgr_ -> log_fp_init("dbsvr.log");
    ReadDbConfig();
    conn = mysql_init(NULL);
    return connectDb();
}

int DbManager::execSql(const string &sql)
{
    num_rows_ = 0;
    num_cols_ = 0;
    if(mysql_query(conn, sql.c_str()))
    {
        if(mysql_ping(conn) == 0)
        {
            mysql_query(conn, "set names utf8");
            return execSql(sql);
        }
        this -> log_mgr_ -> println("query fail : %s \n", mysql_error(conn));
        return DB_CONN_CONNECT_FAIL;
    }
    result = mysql_store_result(conn);
    if(result)
    {
        num_rows_ = mysql_num_rows(result);
        num_cols_ = mysql_num_fields(result);
    }
    return SUCCESS;
}

int DbManager::freeRes()
{
    mysql_free_result(result);
    return SUCCESS;
}

int DbManager::fetch_row()
{
    row = mysql_fetch_row(result);
    return SUCCESS;
}
