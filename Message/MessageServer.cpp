/*************************************************************************
	> File Name: MessageServer.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 27 Jan 2022 10:27:31 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"MessageManager.h"
#include"LogManager.h"
#include"BusManager.h"
#include"common/messsage_type.h"
#include"proto/message_define.pb.h"
#include<unistd.h>
using namespace std;
MessageManager mess_mgr;
static LogManager *log_mgr = LogManager::GetInstance();
static BusManager *bus_mgr = BusManager::GetInstance();
static ssp::ReqInfoBase req_base;
static ssp::RspInfoBase rsp_base;

//回复消息
void Response(int channel_idx)
{
    int ret;
    char send_buf[10240] = {};
    //ret = bus_mgr.get_send_buf(channel_idx, send_buf);
    rsp_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage(channel_idx, send_buf, rsp_base.ByteSizeLong());
    if(ret == SUCCESS) log_mgr -> println("[RelaSvr] Rsp response successed.\n");
    else log_mgr -> println("[RelaSvr] Rsp response failed.\n");
}

void ParseFromArray(const char *buf)
{
    req_base.ParseFromArray(buf, 10239);
}

int main()
{
    log_mgr -> println("MessageServer Start");
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
        log_mgr -> println("get_send_buf:SUCCESS");
        mess_type = req_base.mess_type();
        log_mgr -> println("mess_type : %d", mess_type);
        
        //PublishMessage, GetMessage 
        switch(mess_type)
        {
            case PUBLISH_MESSAGE_REQ:
            {
                const ssp::PublishMessageReq &publish_mess_req = req_base.publish_mess_req();
                ssp::PublishMessageRsp *publish_mess_rsp = rsp_base.mutable_publish_mess_rsp();
                rsp_base.set_mess_type(PUBLISH_MESSAGE_RSP);
                int message_id;
                ret = mess_mgr.PublishMessage(publish_mess_req.user_id(), publish_mess_req.content(), publish_mess_req.publish_time(), message_id);
                log_mgr -> println("ret = [%d]", ret);
                if(ret != SUCCESS)
                {
                    publish_mess_rsp -> set_ret(ret);
                }
                else publish_mess_rsp -> set_ret(message_id);
                Response(channel_idx - 1);
                break;
            }
            case GET_MESSAGE_REQ:
            {
                const ssp::GetMessageReq &get_mess_req = req_base.get_mess_req();
                ssp::GetMessageRsp *get_mess_rsp = rsp_base.mutable_get_mess_rsp();
                rsp_base.set_mess_type(GET_MESSAGE_RSP);
                MessageInfo message_info;
                ret = mess_mgr.GetMessage(get_mess_req.message_id(), message_info);
                log_mgr -> println("GetMessage.ret = [%d]", ret);
                log_mgr -> println("publisher:[%d], message_id:[%d], content[%s], publish_time:[%d]", 
                                    message_info.publisher(), message_info.message_id(), 
                                   message_info.content().c_str(), message_info.publish_time());
                get_mess_rsp -> set_ret(ret);
                if(ret == SUCCESS)
                {
                    message_info.ToPb(get_mess_rsp -> mutable_message_info());
                }
                Response(channel_idx - 1);
                break;
            }
            case MODIFY_MESSAGE_REQ:
            {
                rsp_base.set_mess_type(MODIFY_MESSAGE_RSP);
                const ssp::ModifyMessageReq &modify_mess_req = req_base.modify_mess_req();
                ssp::ModifyMessageRsp *modify_mess_rsp = rsp_base.mutable_modify_mess_rsp();
                int user_id, message_id, modify_time;
                string content;
                user_id = modify_mess_req.user_id();
                message_id = modify_mess_req.message_id();
                modify_time = modify_mess_req.modify_time();
                content = modify_mess_req.content();
                ret = mess_mgr.ModifyMessage(user_id, message_id, modify_time, content);
                modify_mess_rsp -> set_ret(ret);
                Response(channel_idx - 1);
                break;
            }
        }
    }
    return 0;
}

