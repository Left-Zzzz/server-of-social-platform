/*************************************************************************
	> File Name: RelationServer.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 27 Jan 2022 10:02:16 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"RelationManager.h"
#include"LogManager.h"
#include"BusManager.h"
#include"common/messsage_type.h"
#include"proto/message_define.pb.h"
#include<unistd.h>
using namespace std;
RelationManager rela_mgr;
extern LogManager* log_mgr;
static ssp::ReqInfoBase req_base;
static ssp::RspInfoBase rsp_base;
static BusManager *bus_mgr = BusManager::GetInstance();

//回复消息
void Response(int channel_idx)
{
    int ret;
    char send_buf[10240];
    //ret = bus_mgr -> get_send_buf(channel_idx, send_buf);
    rsp_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage(channel_idx, send_buf);
    if(ret == SUCCESS) log_mgr -> println("[RelaSvr] Rsp response successed.\n");
    else log_mgr -> println("[RelaSvr] Rsp response failed with code [%d].\n", ret);
}

void ParseFromArray(const char *buf)
{
    req_base.ParseFromArray(buf, 10239);
}


int main()
{
    log_mgr = LogManager::GetInstance();
    log_mgr -> println("RelationServer Start");
    log_mgr -> println("bus_mgr init.");
    while(1)
    {
        req_base.Clear();
        rsp_base.Clear();
        int channel_idx = -1, ret = -1, mess_type;
        time_t now = time(0);
        char *recv_buf = NULL;
        //轮询，接收队列为空则睡5ms
        while((channel_idx = bus_mgr -> check_recv()) < 0)
            usleep(5000);
        ret = bus_mgr -> TryGetMessage(channel_idx, ParseFromArray, 50, 10);
        if(ret != SUCCESS) continue;
        mess_type = req_base.mess_type();
        log_mgr -> println("mess_type : %d\n", mess_type);
        
        //AddFriend, DeleteFriend, AddBlack, DeleteBlack
        switch(mess_type)
        {
            case ADD_FRIEND_REQ:
            {
                const ssp::AddFriendReq &add_friend_req = req_base.add_friend_req();
                ssp::AddFriendRsp *add_friend_rsp = rsp_base.mutable_add_friend_rsp();
                rsp_base.set_mess_type(ADD_FRIEND_RSP);
                ret = rela_mgr.AddFriend(add_friend_req.user_id(), add_friend_req.other_id());
                add_friend_rsp -> set_ret(ret);
                Response(channel_idx - 1);
                break;
            }
            case GET_FRIENDS_REQ:
            {
                log_mgr -> println("get_friends_req Response.");
                const ssp::GetFriendsReq &get_friends_req = req_base.get_friends_req();
                ssp::GetFriendsRsp *get_friends_rsp = rsp_base.mutable_get_friends_rsp();
                rsp_base.set_mess_type(GET_FRIENDS_RSP);
                RelationInfo relation_ret;
                ret = rela_mgr.GetFriends(get_friends_req.user_id(), &relation_ret);
                for(int i = 0; i < relation_ret.friend_count(); i++)
                {
                    get_friends_rsp -> add_friend_id(relation_ret.friend_list_[i]);
                }
                log_mgr -> println("friends_cnt = [%d]", relation_ret.friend_count());
                Response(channel_idx - 1);
                break;
            }
        }
    }
    return 0;
}

