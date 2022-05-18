/*************************************************************************
	> File Name: RelationInfo.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 02:11:51 PM CST
 ************************************************************************/

#ifndef _RELATIONINFO_H
#define _RELATIONINFO_H
#include"common/macro.h"
#include"proto/relation_info_base.pb.h"
#include"common/ret_value.h"
#include<string.h>
#include<set>
using namespace ssp;

typedef struct RelationInfo
{
public:
    void FromPb(ssp::RelationInfoBase &relation);
    void ToPb(ssp::RelationInfoBase &relation);
private:
    GETSETVAR(int, user_id);
    GETSETVAR(int, friend_count);
    GETSETVAR(int, black_count);
    std::set<int> friend_list_set_;
    std::set<int> black_list_set_;
    int friend_list_[1024];
    int black_list_[1024];
public:
    int GetFriendUserIdByIndex(int index);
    int GetBlackUserIdByIndex(int index);
    int *GetFriendList();
    int *GetBlackList();
    int CheckFriend(int other_id);
    int CheckBlack(int other_id);
    int AddFriend(int other_id);
    int AddBlack(int other_id);
    int DeleteFriend(int other_id);
    int DeleteBlack(int other_id);

}RelationInfo;

#endif
