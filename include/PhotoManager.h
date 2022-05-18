/*************************************************************************
	> File Name: PhotoManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 03:54:49 PM CST
 ************************************************************************/

#ifndef _PHOTOMANAGER_H
#define _PHOTOMANAGER_H
#include"PhotoInfo.h"
#include"common/ret_value.h"
#include"LogManager.h"
#include"string.h"
#include"common/ret_value.h"
#include"DbManager.h"

class PhotoManager
{
private:
    DbManager db_svr;
    static LogManager *log_mgr;
    vector<PhotoInfo> photos_ = vector<PhotoInfo>(10240);
    GETSETVAR(int, photo_count);
public:
    void Start();
    void Proc();
    void Shutdown();
    void Restart();
public:
    PhotoManager();
    ~PhotoManager();
    int SerchPhoto(int user_id, int last_fresh_time);
    int GetPhotos(int user_id, int last_fresh_time, vector<PhotoInfo> &photos, int &photo_count);
    int UpdatePhoto(int user_id, int publisher_id, int publish_time, int publish_message_id);
    int CreatePhoto(int user_id, int publisher_id, int publish_time, int publish_message_id);
};
#endif
