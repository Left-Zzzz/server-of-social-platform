/*************************************************************************
	> File Name: RelationManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 02:26:05 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"common/ret_value.h"
#include"RelationInfo.h"
#include"RelationManager.h"
#include"DbManager.h"
#include"LogManager.h"

LogManager *log_mgr = NULL;
static DbManager db_svr;

RelationManager::RelationManager()
{
    Start();
}

RelationManager::~RelationManager()
{
    Shutdown();
}

void RelationManager::Start()
{
    //初始化日志
    log_mgr = LogManager::GetInstance();
    log_mgr -> log_fp_init("rela_svr.log");
    //初始化数据库
    int ret = db_svr.initDb();
    if(ret != SUCCESS)
    {
        log_mgr -> println("[RelaMgr] db init failed. ret:[%d]", ret);
        exit(1);
    }
    else log_mgr -> println("[RelaMgr] db init successed.");
    log_mgr -> println("RelationManager Start.");
}

void RelationManager::Proc()
{
    log_mgr -> println("RelationManager Proc");
}

void RelationManager::Shutdown()
{
    log_mgr -> println("RelationManager Shutdown.");
}

void RelationManager::Restart()
{
}

int RelationManager::GetFriends(int user_id, RelationInfo *relation_input)
{
    log_mgr -> println("GetFriends");
    int ret;
    //获取好友列表
    string QuerySql = "select other_id from tb_relation_friend where user_id = " + to_string(user_id) + ";";
    ret = db_svr.execSql(QuerySql);
    if(ret != SUCCESS) return ret;
    if(!db_svr.result) return DB_QUERY_FAIL;
    int friend_cnt = 0;
    while(1)
    {
        db_svr.row = mysql_fetch_row(db_svr.result);
        if(!db_svr.row) break;
        relation_input -> friend_list_[friend_cnt++] = atoi(db_svr.row[0]);
    }
    db_svr.freeRes();
    relation_input -> set_friend_count(friend_cnt);

    return SUCCESS;
}

int RelationManager::GetRelation(int user_id, RelationInfo *relation)
{
    log_mgr -> println("GetRelation");
    int ret;
    //获取好友列表
    string QuerySql = "select other_id from tb_relation_friend where user_id = " + to_string(user_id) + ";";
    ret = db_svr.execSql(QuerySql);
    if(ret != SUCCESS) return ret;
    if(!db_svr.result) return DB_QUERY_FAIL;
    int friend_cnt = 0;
    while(1)
    {
        db_svr.row = mysql_fetch_row(db_svr.result);
        if(!db_svr.row) break;
        relation -> friend_list_[friend_cnt++] = atoi(db_svr.row[0]);
    }
    db_svr.freeRes();
    relation -> set_friend_count(friend_cnt);

    //获取黑名单列表
    QuerySql = "select other_id from tb_relation_black where user_id = " + to_string(user_id) + ";";
    ret = db_svr.execSql(QuerySql);
    if(ret != SUCCESS) return ret;
    if(!db_svr.result) return DB_QUERY_FAIL;
    int black_cnt = 0;
    while(1)
    {
        db_svr.row = mysql_fetch_row(db_svr.result);
        if(!db_svr.row) break;
        relation -> black_list_[black_cnt++] = atoi(db_svr.row[0]);
    }
    relation -> set_black_count(black_cnt);
    db_svr.freeRes();
    return SUCCESS;
}

int RelationManager::CreateRelation(int user_id)
{
    return SUCCESS;
}

int RelationManager::CheckFriend(int user_id, int other_id)
{
    log_mgr -> println("CheckFriend");
    std::string Sql = "select * from tb_relation_friend where user_id = " + to_string(user_id) + " and other_id = " + to_string(other_id) + ";";
    int ret = db_svr.execSql(Sql);
    int is_friend_flag = 0;
    if(ret != SUCCESS) return ret;
    if(db_svr.result)
    {
        db_svr.row = mysql_fetch_row(db_svr.result);
        if(db_svr.row && db_svr.row[0]) is_friend_flag = 1;
    }
    db_svr.freeRes();
    log_mgr -> println("CheckFriendFlag:%d, ret:%d", is_friend_flag, ret);
    if(is_friend_flag) return ALREADY_FRIEND;
    return NOT_FRIEND;
}

int RelationManager::CheckBlack(int user_id, int other_id)
{
    log_mgr -> println("CheckBlack");
    std::string Sql = "select * from tb_relation_black where user_id = " + to_string(user_id) + " and other_id = " + to_string(other_id) + ";";
    int ret = db_svr.execSql(Sql);
    int is_black_flag = 0;
    if(ret != SUCCESS) return ret;
    if(db_svr.result)
    {
        db_svr.row = mysql_fetch_row(db_svr.result);
        if(db_svr.row && db_svr.row[0]) is_black_flag = 1;
    }
    db_svr.freeRes();
    log_mgr -> println("CheckBlackFlag:%d, ret:%d", is_black_flag, ret);
    if(is_black_flag) return ALREADY_BLACK;
    return NOT_BLACK;
}

int RelationManager::AddFriend(int user_id,int other_id)
{
    log_mgr -> println("AddFriend");
	int ret;
    RelationInfo relation;
    ret = CheckBlack(user_id, other_id);
    if(ret != NOT_BLACK) return ret;
    ret = CheckFriend(user_id, other_id);
    if(ret != NOT_FRIEND) return ret;
    //ret = GetRelation(user_id, &relation);
    //if(ret != SUCCESS) return ret;
	//if((ret = relation.AddFriend(other_id)) != SUCCESS) return ret;
    //如果成功，回写DB
    string Sql = "insert into tb_relation_friend(user_id, other_id) values(" + to_string(user_id) + ", " + to_string(other_id) + ");";
    ret = db_svr.execSql(Sql);
	return ret;
}

int RelationManager::DeleteFriend(int user_id,int other_id){
    log_mgr -> println("DeleteFriend");
    int ret;
    ret = CheckFriend(user_id, other_id);
    if(ret != ALREADY_FRIEND) return ret;
    //如果是好友，回写DB
    std::string Sql = "delete from tb_relation_friend where user_id = " + to_string(user_id) + " and other_id = " + to_string(other_id) + ";";
    ret = db_svr.execSql(Sql);
	return ret;
}

int RelationManager::AddBlack(int user_id,int other_id){
    log_mgr -> println("AddBlack");
    int ret;
    ret = CheckFriend(user_id, other_id);
    //如果不是朋友，则直接返回
    if(ret == NOT_FRIEND || ret != ALREADY_FRIEND)
        return ret;
    //如果是好友，则先删除好友
    if(ret == ALREADY_FRIEND)
    {
        ret = DeleteFriend(user_id, other_id);
        if(ret != SUCCESS) return ret;
    }
    ret = CheckBlack(user_id, other_id);
    if(ret != NOT_BLACK) return ret;
    //如果非黑名单，回写DB
    string Sql = "insert into tb_relation_black(user_id, other_id) values(" + to_string(user_id) + ", " + to_string(other_id) + ");";
    std::cout << "sql:" << Sql << endl;
    ret = db_svr.execSql(Sql);
	return ret;
}

int RelationManager::DeleteBlack(int user_id,int other_id){
    log_mgr -> println("DeleteBlack");
	int ret;
    ret = CheckBlack(user_id, other_id);
    if(ret != ALREADY_BLACK) return ret;
    //如果是黑名单，回写DB
    string Sql = "delete from tb_relation_black where user_id = " + to_string(user_id) + " and other_id = " + to_string(other_id) + ";";
    ret = db_svr.execSql(Sql);
    return ret;
}
