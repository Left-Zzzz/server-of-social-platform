/*************************************************************************
	> File Name: RelationInfo.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 02:58:51 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"RelationInfo.h"

int RelationInfo::GetFriendUserIdByIndex(int index)
{
    if(index < 0 || index >= friend_count_)
    {
        return USER_NOT_EXIST;
    }
    return friend_list_[index];

}

int RelationInfo::GetBlackUserIdByIndex(int index)
{
    if(index < 0 || index >= black_count_)
    {
        return USER_NOT_EXIST;
    }
    return black_list_[index];
}

//从数据库中获取
int *RelationInfo::GetFriendList()
{
    return SUCCESS;
}
int *RelationInfo::GetBlackList()
{
    return SUCCESS;
}

int RelationInfo::CheckFriend(int other_id)
{
    for(int i = 0; i < friend_count_; i++)
    {
        if(friend_list_[i] == other_id)
            return ALREADY_FRIEND;
    }
    return NOT_FRIEND;
}

int RelationInfo::CheckBlack(int other_id)
{
    for(int i = 0; i < black_count_; i++)
    {
        if(black_list_[i] == other_id)
            return ALREADY_BLACK;
    }
    return NOT_BLACK;
}
int RelationInfo::AddFriend(int other_id)
{
    int check_black_ret = CheckBlack(other_id);
    if(check_black_ret == ALREADY_BLACK)
        return ALREADY_BLACK;

    int check_friend_ret = CheckFriend(other_id);
    if(check_friend_ret == ALREADY_FRIEND)
        return ALREADY_FRIEND;

    if(friend_count_ >= 10239)
    {
        return FRIEND_TOO_MUCH;
    }
    friend_list_[friend_count_] = other_id;
    friend_count_++;
    return SUCCESS;
}
int RelationInfo::AddBlack(int other_id)
{
    int check_friend_ret = CheckFriend(other_id);
    if(check_friend_ret == NOT_FRIEND)
        return NOT_FRIEND;
    DeleteFriend(other_id);
    int check_black_ret = CheckBlack(other_id);
    if(check_black_ret == ALREADY_BLACK)
        return ALREADY_BLACK;
    if(black_count_ >= 10239)
    {
        return BLACK_TOO_MUCH;
    }
    
    black_list_[black_count_] = other_id;
    black_count_++;
    return SUCCESS;
}
int RelationInfo::DeleteFriend(int other_id)
{
    int check_friend_ret = CheckFriend(other_id);
    if(check_friend_ret != ALREADY_FRIEND)
        return check_friend_ret;
    int check_black_ret = CheckBlack(other_id);
    if(check_black_ret != NOT_BLACK)
        return check_black_ret;
    for(int i = 0; i < friend_count_; i++)
    {
        if(friend_list_[i] == other_id)
        {
            friend_list_[i] = -1;
            break;
        }
    }
    return SUCCESS;
}
int RelationInfo::DeleteBlack(int other_id)
{
    int check_friend_ret = CheckFriend(other_id);
    if(check_friend_ret == ALREADY_FRIEND)
        return check_friend_ret;
    int check_black_ret = CheckBlack(other_id);
    if(check_black_ret == NOT_BLACK)
        return check_black_ret;
    for(int i = 0; i < black_count_; i++)
    {
        if(black_list_[i] == other_id)
        {
            black_list_[i] = -1;
            break;
        }
    }
    return SUCCESS;
}


