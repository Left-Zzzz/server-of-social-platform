/*************************************************************************
	> File Name: RelationManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 02:19:50 PM CST
 ************************************************************************/

#ifndef _RELATIONMANAGER_H
#define _RELATIONMANAGER_H
#include"common/macro.h"
#include"RelationInfo.h"
#include"DbManager.h"
class RelationManager
{
private:
    GETSETVAR(int, relation_count);
public:
    void Start();
    void Proc();
    void Shutdown();
    void Restart();
public:
    int CreateRelation(int user_id);
    int GetRelation(int user_id, RelationInfo *relation);
    int GetFriends(int user_id, RelationInfo *relation);
    int CheckFriend(int user_id, int ohter_id);
    int CheckBlack(int user_id, int ohter_id);
    int AddFriend(int user_id,int other_id);
    int DeleteFriend(int user_id,int other_id);
    int AddBlack(int user_id,int other_id);
    int DeleteBlack(int user_id,int other_id);
    RelationManager();
    ~RelationManager();
};
#endif
