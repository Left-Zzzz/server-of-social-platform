/*************************************************************************
    > File Name: main.cpp
    > Author: Left
    > Mail: 1059051242@qq.com
    > Created Time: Sun 09 Jan 2022 01:14:27 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include<unistd.h>
#include"UserManager.h"
#include"RelationManager.h"
#include"MessageManager.h"
#include"PhotoManager.h"
#include"DbManager.h"
#include"mysql/mysql.h"
#include"common/messsage_type.h"
#include"proto/message_define.pb.h"

#include<sys/types.h>          /* See NOTES */
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/file.h>

#define PORT 28887

UserManager user_svr;
//RelationManager relation_svr;
//MessageManager message_svr;
//PhotoManager photo_svr;
extern DbManager db_svr;


ssp::ReqInfoBase req_base;
ssp::RspInfoBase rsp_base;

char send_buffer[10240];
char recv_buffer[10240];
FILE *log_fp;
int svrno = 0;
int sockfd, clnt_fd;

//回复消息
void Response()
{
    rsp_base.SerializeToArray(send_buffer, 10240);
    fprintf(log_fp, "clnt_fd:[%d],发送的size:[%d]\n", clnt_fd, rsp_base.ByteSize());
    send(clnt_fd, send_buffer, rsp_base.ByteSize(), 0);
    fprintf(log_fp, "Rsp response successed.\n");

}

//初始化socket
void SocketInit()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) myperror("socket");
    int ret_status;
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;
    ret_status = bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));
    if(ret_status < 0) myperror("bind");
    ret_status = listen(sockfd, 3);
    if(ret_status < 0) myperror("listen");
}

void *thread_run(void *arg)
{
    string opt;
    while(1)
    {
        std::cin >> opt;
        if(opt == "quit")
            exit(0);
    }
}

void log_fp_init(char *file_path)
{
    //获取服务器no
    FILE *fp = fopen("usersvr_id.ini", "r+");
    if(!fp) myperror("fopen");
    int fd = fileno(fp);
    int ret = flock(fd, LOCK_EX);
    if(ret < 0) myperror("flock");
    fscanf(fp, "%d\n", &svrno);
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%d\n", svrno + 1);
    ret = flock(fd, LOCK_UN);
    if(ret < 0) myperror("flock");
    fclose(fp);
    string file_path_str = to_string(svrno) + "_";
    file_path_str += file_path;
    log_fp = fopen(file_path_str.c_str(), "a+");
    if(!log_fp) myperror("fopen");
    fprintf(log_fp, "svrno:%d\n", svrno);
}

int main()
{
    log_fp_init((char *)"usersvr.log");
    SocketInit();
    fprintf(log_fp, "Socket Listening.\n");
    db_svr.initDb("127.0.0.1", "root", "left1234", "ssp");
    user_svr.Start();
    relation_svr.Start();
    message_svr.Start();
    //photo_svr.Start();
    pthread_t tid;
    pthread_create(&tid, NULL, thread_run, NULL);
    pthread_detach(tid);
    //Test();
    while(1)
    {
        req_base.Clear();
        rsp_base.Clear();
        clnt_fd = accept(sockfd, NULL, NULL);
        int ret_size = recv(clnt_fd, recv_buffer, 10240, 0);
        fprintf(log_fp, "recv data, ret_size = [%d]\n", ret_size);
        time_t now = time(0);
        int ret, mess_type;
        ret = req_base.ParseFromArray(recv_buffer, ret_size);
        mess_type = req_base.mess_type();
        fprintf(log_fp, "mess_type : %d\n", mess_type);
        switch(mess_type)
        {
            case SSP_REG_REQ:
            {
                const ssp::RegReq &reg_req = req_base.reg_req();
                ssp::RegRsp *reg_rsp = rsp_base.mutable_reg_rsp();
                rsp_base.set_mess_type(SSP_REG_RSP);
                ret = user_svr.CreateUser(reg_req.user_name(), reg_req.password(), reg_req.from());
                reg_rsp -> set_ret(ret);
                if(ret == SUCCESS)
                {
                    int user_id = user_svr.GetUserIdByUserName(reg_req.user_name());
                    reg_rsp -> set_user_id(user_id);
                }
                Response();
                break;
            }
            case LOGIN_REQ:
            {
                fprintf(log_fp, "login req\n");
                const ssp::LoginReq &login_req = req_base.login_req();
                ssp::LoginRsp *login_rsp = rsp_base.mutable_login_rsp();
                rsp_base.set_mess_type(LOGIN_RSP);
                ret = user_svr.LoginCheck(login_req.user_name(), login_req.password());
                if(ret != SUCCESS)
                {
                    login_rsp -> set_ret(ret);
                    Response();
                    break;
                }
                int user_id = user_svr.GetUserIdByUserName(login_req.user_name());
                ret = user_svr.UpdateUserLoginTime(user_id, now);
                if(ret != SUCCESS)
                {
                    login_rsp -> set_ret(ret);
                    Response();
                    break;
                }
                login_rsp -> set_user_id(user_id);
                login_rsp -> set_ret(ret);
                Response();
                break;
            }
            case ADD_FRIEND_REQ:
            {
                const ssp::AddFriendReq &add_friend_req = req_base.add_friend_req();
                ssp::AddFriendRsp *add_friend_rsp = rsp_base.mutable_add_friend_rsp();
                rsp_base.set_mess_type(ADD_FRIEND_RSP);
                if(user_svr.CheckExist(add_friend_req.user_id()) == USER_NOT_EXIST || user_svr.CheckExist(add_friend_req.other_id()) == USER_NOT_EXIST)
                {
                    fprintf(log_fp, "user_not_exist.\n");
                    add_friend_rsp -> set_ret(USER_NOT_EXIST);
                }
                else
                {
                    ret = relation_svr.AddFriend(add_friend_req.user_id(), add_friend_req.other_id());
                    add_friend_rsp -> set_ret(ret);
                }
                Response();
                break;
            }
            case ADD_BLACK_REQ:
            {
                fprintf(log_fp, "ADD_BLACK_REQ\n");
                const ssp::AddBlackReq &add_black_req = req_base.add_black_req();
                ssp::AddBlackRsp *add_black_rsp = rsp_base.mutable_add_black_rsp();
                rsp_base.set_mess_type(ADD_BLACK_RSP);
                ret = relation_svr.AddBlack(add_black_req.user_id(), add_black_req.other_id());
                add_black_rsp -> set_ret(ret);
                Response();
                break;
            }
            case DEL_FRIEND_REQ:
            {
                const ssp::DelFriendReq &del_friend_req = req_base.del_friend_req();
                ssp::DelFriendRsp *rsp = rsp_base.mutable_del_friend_rsp();
                rsp_base.set_mess_type(DEL_FRIEND_RSP);
                if(user_svr.CheckExist(del_friend_req.user_id()) == USER_NOT_EXIST || user_svr.CheckExist(del_friend_req.other_id()) == USER_NOT_EXIST)
                {
                    fprintf(log_fp, "user_not_exist.\n");
                    rsp -> set_ret(USER_NOT_EXIST);
                }
                else
                {
                    ret = relation_svr.DeleteFriend(del_friend_req.user_id(), del_friend_req.other_id());
                    rsp -> set_ret(ret);
                }
                Response();
                break;
            }
            case DEL_BLACK_REQ:
            {
                const ssp::DelBlackReq &del_black_req = req_base.del_black_req();
                ssp::DelBlackRsp *rsp = rsp_base.mutable_del_black_rsp();
                rsp_base.set_mess_type(DEL_BLACK_RSP);
                if(user_svr.CheckExist(del_black_req.user_id()) == USER_NOT_EXIST || user_svr.CheckExist(del_black_req.other_id()) == USER_NOT_EXIST)
                {
                    fprintf(log_fp, "user_not_exist.\n");
                    rsp -> set_ret(USER_NOT_EXIST);
                }
                else
                {
                    ret = relation_svr.DeleteBlack(del_black_req.user_id(), del_black_req.other_id());
                    rsp -> set_ret(ret);
                }
                Response();
                break;
            }
            case PUBLISH_MESSAGE_REQ:
            {
                std::cout << "PUBLISH_MESSAGE_REQ" << endl; 
                const ssp::PublishMessageReq &req = req_base.publish_mess_req();
                ssp::PublishMessageRsp *rsp = rsp_base.mutable_publish_mess_rsp();
                rsp_base.set_mess_type(PUBLISH_MESSAGE_RSP);
                int ret_status = message_svr.PublishMessage(req.user_id(), req.content(), now);
                rsp -> set_ret(ret_status);
                Response();
                break;   
            }       
            
            case GET_PHOTO_REQ:
            {
                const ssp::GetPhotoReq &req = req_base.get_photo_req();
                ssp::GetPhotoRsp *rsp = rsp_base.mutable_get_photo_rsp();
                rsp_base.set_mess_type(GET_PHOTO_RSP);
                PhotoInfo photo_infos[10240];
                int ret_size;
                int ret = photo_svr.GetPhotos(req.user_id(), now, photo_infos, ret_size);
                if(ret != SUCCESS)
                {
                    rsp -> set_ret(ret);
                    Response();
                    break;
                }
                fprintf(log_fp, "ret_size的值为：%d\n", ret_size);
                for(int i = 0; i < ret_size; i++)
                {
                    ssp::MessageBase *message_info = rsp -> add_message_info();
                    message_info -> set_publisher(photo_infos[i].last_publisher());
                    message_info -> set_publish_time(photo_infos[i].last_publish_time());
                    message_info -> set_message_id(photo_infos[i].last_publish_message_id());
                }
                fprintf(log_fp, "ret_photos_size = [%d]\n", ret_size);
                rsp -> set_ret(SUCCESS);
                fprintf(log_fp, "rsp -> ret = [%d]\n", rsp -> ret());
                fprintf(log_fp, "rsp -> message_info_size = [%d]\n", rsp -> message_info_size());
                Response();
                break;
            }
            
            case GET_MESSAGE_REQ:
            {
                const ssp::GetMessageReq &req = req_base.get_mess_req();
                ssp::GetMessageRsp *rsp = rsp_base.mutable_get_mess_rsp();
                rsp_base.set_mess_type(GET_MESSAGE_RSP);
                //获取消息列表
                MessageInfo *message = message_svr.GetMessage(req.message_id());
                if(message == NULL)
                {
                    rsp -> set_ret(MESSAGE_NOT_EXIST);
                }
                else
                {
                    ssp::MessageBase *message_base = rsp -> mutable_message_info();
                    message_base -> set_content(message -> content());
                    rsp -> set_ret(SUCCESS);
                }
                Response();
                break;
            }
            case DELETE_MESSAGE_REQ:
            {
                const ssp::DeleteMessageReq &req = req_base.delete_mess_req();
                ssp::DeleteMessageRsp *rsp = rsp_base.mutable_delete_mess_rsp();
                rsp_base.set_mess_type(DELETE_MESSAGE_RSP);
                int user_id = req.user_id();
                int message_id = req.message_id();
                int ret = message_svr.DeleteMessage(message_id);
                rsp -> set_ret(SUCCESS);
                Response();
                break;
            }
            case LOGOUT_REQ:
            {
                const ssp::LogoutReq &req = req_base.logout_req();
                ssp::LogoutRsp *rsp = rsp_base.mutable_logout_rsp();
                rsp_base.set_mess_type(LOGOUT_RSP);
                ret = user_svr.CheckExist(req.user_id());
                if(ret == USER_EXIST)
                {
                    ret = user_svr.Logout(req.user_id(), now);
                    rsp -> set_ret(ret);
                }
                else
                {
                    rsp -> set_ret(ret);
                }
                Response();
                break;
            }
            case DELETE_USER_REQ:
            {
                const ssp::DeleteUserReq &req = req_base.delete_user_req();
                ssp::DeleteUserRsp *rsp = rsp_base.mutable_delete_user_rsp();
                rsp_base.set_mess_type(DELETE_USER_RSP);
                int ret = user_svr.DeleteUser(req.user_id());
                rsp -> set_ret(ret);
                Response();
                break;
            }
        }
        mess_type = -1;
        user_svr.Proc();
        relation_svr.Proc();
        message_svr.Proc();
        photo_svr.Proc();
        usleep(5000); //5ms
        fprintf(log_fp, "连接关闭\n");
        close(clnt_fd);
    }
    user_svr.Shutdown();
    relation_svr.Shutdown();
    message_svr.Shutdown();
    photo_svr.Shutdown();
    return 0;
}
