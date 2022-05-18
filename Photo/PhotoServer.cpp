/*************************************************************************
	> File Name: PhotoServer.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 27 Jan 2022 10:40:30 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"PhotoManager.h"
#include"LogManager.h"
#include"BusManager.h"
#include"common/messsage_type.h"
#include"proto/message_define.pb.h"
#include<unistd.h>
using namespace std;
PhotoManager photo_mgr;
static LogManager *log_mgr = LogManager::GetInstance();
static BusManager *bus_mgr = BusManager::GetInstance();
static ssp::ReqInfoBase req_base;
static ssp::RspInfoBase rsp_base;

//回复消息
void Response(int channel_idx)
{
    int ret;
    char send_buf[10240];
    //ret = bus_mgr.get_send_buf(channel_idx, send_buf);
    rsp_base.SerializeToArray(send_buf, 10239);
    ret = bus_mgr -> SendMessage(channel_idx, send_buf);
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
            case CREATE_PHOTO_REQ:
            {
                const ssp::CreatePhotoReq &create_photo_req = req_base.create_photo_req();
                ssp::CreatePhotoRsp *create_photo_rsp = rsp_base.mutable_create_photo_rsp();
                rsp_base.set_mess_type(CREATE_PHOTO_RSP);
                for(int i = 0; i < create_photo_req.user_id_size(); i++)
                {
                    ret = photo_mgr.CreatePhoto(create_photo_req.user_id(i), create_photo_req.publisher_id(), 
                                                create_photo_req.time(), create_photo_req.message_id());
                    if(ret != SUCCESS) break;
                }
                create_photo_rsp -> set_ret(ret);
                Response(channel_idx - 1);
                break;
            }
        }
    }
    return 0;
}
