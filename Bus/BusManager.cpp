/*************************************************************************
	> File Name: BusManager.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Fri 21 Jan 2022 05:52:01 PM CST
 ************************************************************************/

#include<sys/ipc.h>
#include<sys/shm.h>
#include<bits/stdc++.h>
#include<unistd.h>
#include"BusManager.h"
#include"common/messsage_type.h"
#include"common/ret_value.h"
#include"common/macro.h"
using namespace std;

LogManager *BusManager::log_mgr = LogManager::GetInstance();
BusManager *BusManager::instance_ = BusManager::GetInstance();

BusManager *BusManager::GetInstance()
{
    if(instance_) return instance_;
    instance_ = new BusManager(0, (char *)"./bus.ini");
    return instance_;
}

int BusManager::SendMessage(std::string send_channel_name, const char *message)
{
    if(!channel_name.count(send_channel_name))
    {
        log_mgr -> println("invalid_name:[%s]", send_channel_name);
        return BUS_INVALID_CHANNEL_NAME;
    }
    return SendMessage(channel_name[send_channel_name], message);
}

//给指定channel发送消息
int BusManager::SendMessage(int send_channel_idx, const char *message)
{
    int ret = 0;
    char *buf = NULL;
    if(ret = get_send_buf(send_channel_idx, buf))
    {
        return ret;
    }
    strcpy(buf, message);
    fresh_next_send(send_channel_idx);
    return SUCCESS;
}

int BusManager::SendMessage(std::string send_channel_name, const char *message, int message_len)
{
    if(!channel_name.count(send_channel_name))
    {
        log_mgr -> println("invalid_name:[%s]", send_channel_name);
        return BUS_INVALID_CHANNEL_NAME;
    }
    return SendMessage(channel_name[send_channel_name], message, message_len);
    
}

//给指定channel发送消息
int BusManager::SendMessage(int send_channel_idx, const char *message, int message_len)
{
    int ret = 0;
    char *buf = NULL;
    if(ret = get_send_buf(send_channel_idx, buf))
    {
        return ret;
    }
    memcpy(buf, message, message_len);
    fresh_next_send(send_channel_idx);
    return SUCCESS;
}

int BusManager::GetResponse(std::string recv_channel_name, char *recv_buf)
{
    if(!channel_name.count(recv_channel_name))
    {
        log_mgr -> println("invalid_name:[%s]", recv_channel_name);
        return BUS_INVALID_CHANNEL_NAME;
    }
    return GetResponse(channel_name[recv_channel_name], recv_buf);
    
}

//往指定channel阻塞获取回复
int BusManager::GetResponse(int recv_channel_idx, char *recv_buf)
{
    int ret = 0;
    char *buf = NULL;
    while(ret = get_recv_buf(recv_channel_idx, buf))
    {
        usleep(200);
    }
    return SUCCESS;
    
}

int BusManager::TryGetMessage(std::string recv_channel_name, char *recv_buf, 
                              int message_len, int wait_time, int try_cnt)
{
    if(!channel_name.count(recv_channel_name))
    {
        log_mgr -> println("invalid_name:[%s]", recv_channel_name);
        return BUS_INVALID_CHANNEL_NAME;
    }
    return TryGetMessage(channel_name[recv_channel_name], recv_buf, 
                         message_len, wait_time, try_cnt);
    
}

int BusManager::TryGetMessage(int recv_channel_idx, char *recv_buf, 
                              int message_len, int wait_time, int try_cnt)
{
    log_mgr -> println("channel = [%d]", recv_channel_idx);
    int ret = 0;
    char *buf = NULL;
    while(ret = get_recv_buf(recv_channel_idx, buf))
    {
        --try_cnt;
        if(try_cnt <= 0) return ret;
        usleep(wait_time);
    }
    memcpy(recv_buf, buf, message_len);
    return fresh_next_recv(recv_channel_idx);
}

int BusManager::TryGetMessage(std::string recv_channel_name, 
                              void(*func)(const char *recv_buf), int wait_time, int try_cnt)
{
    if(!channel_name.count(recv_channel_name))
    {
        log_mgr -> println("invalid_name:[%s]", recv_channel_name);
        return BUS_INVALID_CHANNEL_NAME;
    }
    return TryGetMessage(channel_name[recv_channel_name], func, wait_time, try_cnt);
    
}

int BusManager::TryGetMessage(int recv_channel_idx, void(*func)(const char* recv_buf), 
                              int wait_time, int try_cnt)
{
    log_mgr -> println("channel = [%d]", recv_channel_idx);
    int ret = 0;
    char *buf = NULL;
    while(ret = get_recv_buf(recv_channel_idx, buf))
    {
        --try_cnt;
        if(try_cnt <= 0) return BUS_GET_BUF_FAIL;
        usleep(wait_time);
    }
    if(func)
    {
        func(buf);
    }
    fresh_next_recv(recv_channel_idx);
    return SUCCESS;
}

void BusManager::read_config(char *filename)
{
    FILE* fp = fopen("busconfig.ini", "r");
    if(!fp)
    {
        log_mgr -> println("busconfig.ini配置文件读取失败!");
        exit(1);
    }
    fscanf(fp, "channel_count=%d\n", &channel_count_);
    fscanf(fp, "singal_channel_size=%d\n", &singal_channel_size_);
    fscanf(fp, "queue_count=%d\n", &queue_count_);
    log_mgr -> println("channel_count=%d", channel_count_);
    log_mgr -> println("singal_channel_size=%d", singal_channel_size_);
    queue_size_ = singal_channel_size_;
    singal_channel_size_ = sizeof(head_) + queue_count_ * queue_size_;
    log_mgr -> println("queue_count=%d", queue_count_);
    fclose(fp);

    fp = fopen(filename, "r");
    if(!fp)
    {
        log_mgr -> println("共享内存信道配置文件读取失败!");
        exit(1);
    }
    int read_channel_idx = -1, write_channel_idx = -1;
    char channel_name_read[1024], channel_name_write[1024];
    log_mgr -> println("strt read bus config.");

    while(~fscanf(fp, "read_channel_idx=%d:%s\nwrite_channel_idx=%d:%s\n", 
                  &read_channel_idx, channel_name_read, &write_channel_idx, channel_name_write))
    {
        log_mgr -> println("read_channel_idx=%d:%s\nwrite_channel_idx=%d:%s", 
                           read_channel_idx, channel_name_read, write_channel_idx, channel_name_write);
        if(read_channel_idx < 0 || write_channel_idx < 0)
            break;
        read_channels.push_back(read_channel_idx);
        write_channels.push_back(write_channel_idx);
        channel_name[string(channel_name_read)] = read_channel_idx;
        channel_name[string(channel_name_write)] = write_channel_idx;
    }
}

//得到信道头
Head *BusManager::get_channel_head(int channel_idx)
{
    return (Head *)((char *)shmp_ + channel_idx * singal_channel_size_);
}

//得到消息体buf
char *BusManager::get_channel_buf(Head *head_)
{
    return (char *)head_ + sizeof(head_) + head_ -> tail * queue_size_;
}

//to:send_idx
//todo:负载均衡
int BusManager::get_send_buf(int send_channel_idx, char *&send_buf_)
{
    send_buf_ = NULL;
    head_ = get_channel_head(send_channel_idx);
    int send_idx = head_ -> head;
    char *buf = get_channel_buf(head_);
    if(!buf) return BUS_GET_BUF_FAIL;
    if(*buf) return BUS_QUEUE_FULL;
    send_buf_ = buf + 1;
    log_mgr -> println("get_send_buf, buf:[%p]", send_buf_);
    return SUCCESS;
}

int BusManager::fresh_next_send(int send_channel_idx)
{
    head_ = get_channel_head(send_channel_idx);
    int send_idx = head_ -> head;
    char *buf = get_channel_buf(head_);
    if(!buf) return BUS_GET_BUF_FAIL;
    if(*buf) return BUS_QUEUE_FULL;
    *buf = 'A';
    head_ -> head = (head_ -> head + 1) % queue_count_;
    return SUCCESS;
}

int BusManager::get_recv_buf(int recv_channel_idx, char *&recv_buf_)
{
    recv_buf_ = NULL;
    head_ = (Head *)((char *)shmp_ + recv_channel_idx * singal_channel_size_);
    int recv_idx = head_ -> tail;
    char *buf = get_channel_buf(head_);
    if(!buf) return BUS_GET_BUF_FAIL;
    if(!*buf) return BUS_QUEUE_EMPTY;
    recv_buf_ = buf + 1;
    log_mgr -> println("get_recv_buf, buf:[%p]", recv_buf_);
    return SUCCESS;
}

int BusManager::fresh_next_recv(int recv_channel_idx)
{
    head_ = (Head *)((char *)shmp_ + recv_channel_idx * singal_channel_size_);
    int recv_idx = head_ -> tail;
    char *buf = (char *)head_ + sizeof(head_) + recv_idx * queue_size_;
    if(!buf) return BUS_GET_BUF_FAIL;
    if(!*buf) return BUS_QUEUE_EMPTY;
    memset(buf, 0, queue_size_);
    *buf = 0;
    head_ -> tail = (head_ -> tail + 1) % queue_count_;
    return SUCCESS;
}


//初始化信道
void BusManager::init_channel(short svr_type)
{
    for(int i = 0; i < read_channels.size(); i++)
    {
        int read_channel_idx = read_channels[i];
        head_ = (Head *)((char *)shmp_ + read_channel_idx * singal_channel_size_);
        if(!head_ -> recv_svr_type)
        {
            head_ -> recv_svr_type = svr_type;
            log_mgr -> println("one read channel used.");
        }
    }

    for(int i = 0; i < write_channels.size(); i++)
    {
        int write_channel_idx = write_channels[i];
        head_ = (Head *)((char *)shmp_ + write_channel_idx * singal_channel_size_);
        if(!head_ -> send_svr_type)
        {
            head_ -> send_svr_type = svr_type;
            log_mgr -> println("one write channel used.");
        }
    }
    
}

int BusManager::init_shm()
{
    int shm_id = shmget((key_t)8998, singal_channel_size_ * channel_count_, IPC_CREAT | IPC_EXCL | 0666);
    if(shm_id < 0)
    {
        shm_id = shmget((key_t)8998, singal_channel_size_ * channel_count_, IPC_CREAT | 0666);
        shmp_ = (char *)shmat(shm_id, NULL, 0);
        if(shmp_ < 0) myperror("shmat");
    }
    else
    {
        shmp_ = (char *)shmat(shm_id, NULL, 0);
        if(shmp_ < 0) myperror("shmat");
        memset(shmp_, 0, 10240 * 12);
    }
    return SUCCESS;
}

//查找recv_channel中有无消息可读
//返回：channel_idx
int BusManager::check_recv()
{
    for(int i = 0; i < read_channels.size(); i++)
    {
        head_ = get_channel_head(read_channels[i]);
        char *buf = get_channel_buf(head_);
        if(*buf) return read_channels[i];
    }
    return BUS_CHECK_RECV_EMPTY;
}

BusManager::BusManager()
{
    BusManager(BUS_UNDEFINED_TYPE, (char*)"bus.ini");
}

BusManager::BusManager(short bus_channel_type)
{
    BusManager(bus_channel_type, (char*)"bus.ini");
}

BusManager::BusManager(short bus_channel_type, char *filepath)
{

    log_mgr = LogManager::GetInstance();
    log_mgr -> log_fp_init("bus.log");
    read_config(filepath);
    log_mgr -> println("正在初始化共享内存.");
    init_shm();
    log_mgr -> println("正在初始化信道.");
    init_channel(bus_channel_type);
    log_mgr -> println("BusManager 初始化成功！");
}
