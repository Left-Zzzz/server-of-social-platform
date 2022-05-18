/*************************************************************************
	> File Name: PhotoManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 04:05:12 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"PhotoManager.h"

LogManager *PhotoManager::log_mgr = NULL;

int PhotoManager::SerchPhoto(int user_id, int last_fresh_time)
{
    int ret;

    //todo:验证用户操作交给UserSvr做，PhotoMgr只管对Photo操作
    //if((ret = user_svr.CheckExist(user_id)) != USER_EXIST) return ret;

    std::string Sql = "select * from tb_photo where user_id = " + to_string(user_id) + " and publish_time >= " + to_string(last_fresh_time) + ";";
    if(db_svr.execSql(Sql) == DB_QUERY_FAIL || db_svr.result == NULL) return DB_QUERY_FAIL;

    int num_rows = min(db_svr.num_rows(), 10240);

    for(int i = 0; i < num_rows; i++)
    {
        db_svr.fetch_row();
        //1.user_id 2.publisher_id 3.publish_time 4.publish_mess_id
        photos_[i].set_user_id(atoi(db_svr.row[0]));
        photos_[i].set_last_publisher(atoi(db_svr.row[1]));
        photos_[i].set_last_publish_time(atoi(db_svr.row[2]));
        photos_[i].set_last_publish_message_id(atoi(db_svr.row[3]));
    }
    db_svr.freeRes();

    photo_count_ = num_rows;

    return SUCCESS;
}

int PhotoManager::GetPhotos(int user_id, int last_fresh_time, vector<PhotoInfo> &photos, int &photo_count)
{
    int ret;
    if(ret = (SerchPhoto(user_id, last_fresh_time) != SUCCESS)) return ret;
    photos = photos_;
    photo_count = photo_count_;
    //todo：更改用户刷新时间由UserSvr处理
    //user_svr.UpdateFreshTime(user_id, fresh_time);
    return SUCCESS;
}

int PhotoManager::CreatePhoto(int user_id, int publisher_id, int publish_time, int publish_message_id)
{
    int ret;
    //todo:交由UserSvr查询user_id合法性
    //if((ret = user_svr.CheckExist(user_id)) != USER_EXIST) return ret;

    //将Photo插入到表中
    std::string Sql = "insert into tb_photo values(" + to_string(user_id) + ", " + to_string(publisher_id) + ", " + to_string(publish_time) + ", " + to_string(publish_message_id) + ");";
    ret = db_svr.execSql(Sql);
    
    return ret;
}

int PhotoManager::UpdatePhoto(int user_id, int publisher_id, int publish_time, int publish_message_id)
{
    if(user_id <= 0) return USER_NOT_EXIST;


    return SUCCESS;
}

void PhotoManager::Start()
{
    log_mgr = LogManager::GetInstance();
    log_mgr -> log_fp_init("photo.log");
    int ret;
    //初始化数据库,从配置文件里读
    ret = db_svr.initDb();
    if(ret != SUCCESS)
    {
        printf("[PhotoMgr] init db failed. ret:[%d]\n", ret);
        exit(1);
    }
    else printf("[PhotoMgr] init db successed.\n");
    printf("PhotoManager Start.\n");
    
}
void PhotoManager::Proc()
{

}
void PhotoManager::Shutdown()
{
    printf("PhotoManager Shutdown.\n");
    
}
void PhotoManager::Restart()
{

}

PhotoManager::PhotoManager()
{
    Start();
}
PhotoManager::~PhotoManager()
{
    Shutdown();
}


