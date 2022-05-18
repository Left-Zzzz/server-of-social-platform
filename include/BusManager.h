/*************************************************************************
	> File Name: BusManager.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Thu 27 Jan 2022 03:28:54 PM CST
 ************************************************************************/

#ifndef _BUSMANAGER_H
#define _BUSMANAGER_H
#include"common/macro.h"
#include"LogManager.h"
struct Head
{
    short recv_svr_type;
    short send_svr_type;
    short head;
    short tail;
};

class BusManager
{
private:
    std::vector<int> read_channels;
    std::vector<int> write_channels;
    GETSETPTR(char, recv_buf)
    GETSETPTR(char, send_buf)
    GETSETPTR(char, shmp)
    GETSETPTR(Head, head)
    GETSETVAR(int, channel_count)
    GETSETVAR(int, singal_channel_size)
    GETSETVAR(int, queue_size)
    GETSETVAR(int, channel_head_size)
    GETSETVAR(int, queue_count)

private:
    std::unordered_map<std::string, int> channel_name;
    static BusManager *instance_;
    static LogManager *log_mgr;
    Head *get_channel_head(int channel_idx);
    char *get_channel_buf(Head *head_);
    int init_shm();
    void init_channel(short svr_type);
    void read_config(char *filename);
    int get_send_buf(int send_channel_idx, char *&send_buf_);
    int fresh_next_send(int send_channel_idx);
    int get_recv_buf(int recv_channel_idx, char *&recv_buf_);
    int fresh_next_recv(int recv_channel_idx);
    BusManager();
    BusManager(short bus_channel_type);
    BusManager(short bus_channel_type, char *filepath);
    BusManager(BusManager&) = delete;
    BusManager &operator=(BusManager&) = delete;
public:
    int SendMessage(std::string send_channel_name, const char *message);
    int SendMessage(std::string send_channel_name, const char *message, int message_len);
    int GetResponse(std::string recv_channel_name, char *recv_buf);
    int TryGetMessage(std::string recv_channel_name, char *recv_buf, int message_len, int wait_time, int try_cnt);
    int TryGetMessage(std::string recv_channel_name, void(*func)(const char *recv_buf), int wait_time, int try_cnt);
    int SendMessage(int send_channel_idx, const char *message);
    int SendMessage(int send_channel_idx, const char *message, int message_len);
    int GetResponse(int recv_channel_idx, char *recv_buf);
    int TryGetMessage(int recv_channel_idx, char *recv_buf, int message_len, int wait_time, int try_cnt);
    int TryGetMessage(int recv_channel_idx, void(*func)(const char *recv_buf), int wait_time, int try_cnt);
    int check_recv();
    static BusManager *GetInstance();
};

#endif
