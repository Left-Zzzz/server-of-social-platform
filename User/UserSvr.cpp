/*************************************************************************
	> File Name: UserSvr.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 30 Jan 2022 09:13:21 PM CST
 ************************************************************************/

#include<bits/stdc++.h>
#include"UserManager.cpp"
using namespace std;
FILE *log_fp;

void log_fp_init(char *file_path)
{
    int svrno;
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
    fprintf(log_fp, "svrno:%d\n", svrno),fflush(log_fp);
}

int main()
{
    UserManager mgr;
    return 0;
}
