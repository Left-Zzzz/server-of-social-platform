#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
#include<rapidjson/writer.h>
#include<bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>

#include"proto/message_define.pb.h"
#include "common/messsage_type.h"
#include "fcgi/fcgio.h"
#include "fcgi/fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "ResponseManager.h"
#include "UserManager.h"
#include <sys/file.h>
#include "LogManager.h"
#include "BusManager.h"
#include "token_lib.cpp"
#include "RelationInfo.h"
extern char ** environ;
int token_expire_time_sec = 14400;
using namespace std;
using namespace rapidjson;

void log_fp_init(LogManager &log_mgr,char *file_path)
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
    log_mgr.log_fp_init(file_path_str.c_str());
    log_mgr.println("svrno:%d\n", svrno);
}

int cut_cookies(vector<string> &vec, char *cookie)
{
    int cnt = 0;
    char *p = strtok(cookie, ";");
    if(!p) return cnt;
    string temp = p;
    vec.push_back(temp);
    ++cnt;
    while(p = strtok(NULL, ";"))
    {
        if(*p == ' ') temp = p + 1;
        else temp = p;
        vec.push_back(temp);
        ++cnt;
    }
    return cnt;
}

int urldecode(char* encd, char* decd)
{
    int j,i;
    char *cd =(char*) encd;
    char p[2];
    unsigned int num;
    j=0;

    for( i = 0; i < strlen(cd); i++ )
    {
        memset( p, '\0', 2 );
        if( cd[i] != '%' )
        {
            decd[j++] = cd[i];
            continue;
        }

        p[0] = cd[++i];
        p[1] = cd[++i];

        p[0] = p[0] - 48 - ((p[0] >= 'A') ? 7 : 0) - ((p[0] >= 'a') ? 32 : 0);
        p[1] = p[1] - 48 - ((p[1] >= 'A') ? 7 : 0) - ((p[1] >= 'a') ? 32 : 0);
        decd[j++] = (unsigned char)(p[0] * 16 + p[1]);

    }
    decd[j] = '\0';

    return 0;
}
static const unsigned long STDIN_MAX = 1000000;

static long gstdin(FCGX_Request * request, char ** content)
{
    char * clenstr = FCGX_GetParam("CONTENT_LENGTH", request->envp);
    unsigned long clen = STDIN_MAX;

    if (clenstr)
    {
        clen = strtol(clenstr, &clenstr, 10);
        if (*clenstr)
        {
            cerr << "can't parse \"CONTENT_LENGTH="
                 << FCGX_GetParam("CONTENT_LENGTH", request->envp)
                 << "\"\n";
            clen = STDIN_MAX;
        }

        // *always* put a cap on the amount of data that will be read
        if (clen > STDIN_MAX) clen = STDIN_MAX;

        *content = new char[clen + 1];
        //这里临时设置为0作为结束标志，以后遇到读其他二进制再改回来，暂时只读文本
        (*content)[clen] = 0;
        cin.read(*content, clen);
        clen = cin.gcount();

    }
    else
    {
        // *never* read stdin when CONTENT_LENGTH is missing or unparsable
        *content = 0;
        clen = 0;
    }

    // Chew up any remaining stdin - this shouldn't be necessary
    // but is because mod_fastcgi doesn't handle it correctly.

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do cin.ignore(1024); while (cin.gcount() == 1024);

    return clen;
}

string get_json(int code, const char *message, const char *value_name, Value &value)
{
    Document doc;
    Document::AllocatorType &alloc = doc.GetAllocator();
    doc.SetObject();
    doc.AddMember("code", code, alloc);
    Value mess_val;
    mess_val = StringRef(message);
    doc.AddMember("message", mess_val, alloc);
    if(strcmp(value_name, ""))
    {
        Value value_info;
        value_info = StringRef(value_name);
        doc.AddMember(value_info, value, alloc);
    }
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

//解析Url到map中
void ParseUrl(char *content, unordered_map<string, string> &map)
{
    if(!content) return;
    //临时储存键值对信息
    vector<char *> kv_temp;
    char *p, *k, *v;
    p = strtok(content, "&");
    if(!p) return;
    kv_temp.push_back(p);
    while(p = strtok(NULL, "&"))
        kv_temp.push_back(p);
    for(auto p1 : kv_temp)
    {
        k = strtok(p1, "=");
        if(!k) continue;
        v = strtok(NULL, "=");
        if(!v) continue;
        map[k] = v;
    }
}

int main (void)
{
    BusManager bus_mgr(BUS_USER_TYPE, (char *)"bus.ini");
    LogManager log_mgr;
    log_fp_init(log_mgr, (char *)"websvr.log");
    UserManager user_mgr;
    TokenManager *token_mgr = TokenManager::GetInstance();
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0)
    {
        ssp::ReqInfoBase req_base;
        ssp::RspInfoBase rsp_base;
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

#if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
        cin  = &cin_fcgi_streambuf;
        cout = &cout_fcgi_streambuf;
        cerr = &cerr_fcgi_streambuf;
#else
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
#endif

        // Although FastCGI supports writing before reading,
        // many http clients (browsers) don't support it (so
        // the connection deadlocks until a timeout expires!).
        ResponseManager rsp;
        Document doc;
        //储存解析后的表单数据
        unordered_map<string, string> form_data;
        //储存Url请求解析后的数据
        unordered_map<string, string> req_data;
        char *content;
        char content_decode[1024] = {0};
        //获取cookie并拆分
        char *cookie = FCGX_GetParam("HTTP_COOKIE", request.envp);
        vector<string> cookie_vec;
        int ret_cnt;
        if(cookie) ret_cnt = cut_cookies(cookie_vec, cookie);
        unsigned long clen = gstdin(&request, &content);
        //根据请求方式处理请求
        char *temp = FCGX_GetParam("REQUEST_METHOD", request.envp);
        string req_method;
        if(temp) req_method = temp;

        temp = FCGX_GetParam("SCRIPT_NAME", request.envp);
        string req_script;
        if(temp) req_script = temp;
        time_t now = time(0);
        //对请求作相应
        if(req_method == "GET")
        {
            char query_string[512], query_decode[512];
            temp = FCGX_GetParam("QUERY_STRING", request.envp);
            if(temp)
            {
                strcpy(query_string, temp);
                urldecode(query_string, query_decode);
            }
        }
        else if(req_method == "POST")
        {
            //conten:POST收到的内容，clen：收到内容大小

            string content_type;
            temp = FCGX_GetParam("CONTENT_TYPE", request.envp);    
            if(temp) content_type = temp;
            log_mgr.println("%s", content);
            if(content_type == "application/json")
            {
                //解析json
                log_mgr.println("解析json");
                doc.Parse<kParseStopWhenDoneFlag>(content);
            }
            else 
            {
                urldecode(content, content_decode);
                ParseUrl(content_decode, form_data);
            }
        }

        Value value;
        if(req_script == "/api/login")
        {
            if(req_method == "POST")
            {
                //这里应该是查表单
                if(!form_data.count("user_name") || !form_data.count("password"))
                {
                    rsp.set_content(get_json(-1, "请求的信息错误", "", value));
                }
                //成功处理
                else
                {
                    int ret = -1;
                    log_mgr.println("user_name = [%s], password = [%s]\n", form_data["user_name"].c_str(), form_data["password"].c_str());
                    
                    ret = user_mgr.LoginCheck(form_data["user_name"], form_data["password"]);
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "登录失败", "", value));
                    }
                    //todo：调用UserSvr的Login，成功的话设置Cookie
                    else
                    {
                        int user_id = user_mgr.GetUserIdByUserName(form_data["user_name"]);
                        value = user_id;
                        rsp.set_content(get_json(0, "登录成功", "user_id", value));
                        //todo:设置cookie
                        string user_id_str = to_string(user_id);
                        string expire_str = to_string(time(0) + token_expire_time_sec);
                        string ret_token = token_mgr -> CreateToken(user_id_str, expire_str);
                        rsp.set_cookie(ret_token);
                    }
                }


                /*
                //json获取解析数据方法
                
                if(!doc.IsObject() || !doc.HasMember("user_id") || !doc["user_id"].IsInt())
                {
                    const char *mess = "请求的信息错误", *value_title = "";
                    //写json
                    rsp.set_content(get_json(-1, mess,  value_title, value));
                }
                else
                {
                    const char *mess = "登录成功", *value_title = "";
                    //写json
                    rsp.set_content(get_json(0, mess,  value_title, value));
                }
                */
                
            }
            else if(req_method == "GET")
            {
                rsp.set_content(get_json(-1, "请求方法暂不可用", "", value));
                rsp.set_status("400");
            }

        }
        else if(req_script == "/api/reg")
        {
            if(req_method == "POST")
            {
                //这里应该是查表单
                if(!form_data.count("user_name") || !form_data.count("password"))
                {
                    rsp.set_content(get_json(-1, "请求的信息错误", "", value));
                }
                //成功处理
                else
                {
                    int ret = -1;
                    ret = user_mgr.CreateUser(form_data["user_name"], form_data["password"], 0);
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "注册失败", "", value));
                    }
                    //todo：调用UserSvr的Login，成功的话设置Cookie
                    else
                    {
                        rsp.set_content(get_json(0, "注册成功", "", value));
                    }
                }
            }
            else if(req_method == "GET")
            {
                rsp.set_content(get_json(-1, "请求方法暂不可用", "", value));
                rsp.set_status("400");
            }
        }
        else if(req_script == "/api/addfriend")
        {
            if(req_method == "POST")
            {
                //这里应该是查表单
                if(!doc.HasMember("user_id") || !doc.HasMember("other_id") 
                   || !doc["user_id"].IsInt() || !doc["other_id"].IsInt())
                {
                    rsp.set_content(get_json(-1, "请求的信息错误", "", value));
                }
                //成功处理
                else
                {
                    int ret = -1;
                    ret = user_mgr.CheckExist(doc["user_id"].GetInt()) & user_mgr.CheckExist(doc["other_id"].GetInt());
                    if(ret != USER_EXIST)
                    {
                        rsp.set_content(get_json(ret, "用户不存在", "", value));
                        goto RESPONSE;
                    }
                    char *send_buf = NULL, *recv_buf = NULL;
                    int wait_cnt = 10;
                    ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_send_buf(1, send_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "send_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }
                    //写请求到Bus
                    ssp::AddFriendReq *pbreq = req_base.mutable_add_friend_req();
                    req_base.set_mess_type(ADD_FRIEND_REQ);
                    pbreq -> set_user_id(doc["user_id"].GetInt());
                    pbreq -> set_other_id(doc["other_id"].GetInt());
                    log_mgr.println("mess_type : %d\n", req_base.mess_type());
                    req_base.SerializeToArray(send_buf, 10239);
                    log_mgr.println("ret_size = [%d]\n", req_base.ByteSizeLong());
                    bus_mgr.fresh_next_send(1);
                    int cnt = 6;
                    ret = -1;
                    while(cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_recv_buf(0, recv_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "recv_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }
                    rsp_base.ParseFromArray(recv_buf, 10239);
                    bus_mgr.fresh_next_recv(0);
                    ssp::AddFriendRsp *pbrsp = rsp_base.mutable_add_friend_rsp();
                    ret = pbrsp -> ret();
                    if(ret == SUCCESS)
                        rsp.set_content(get_json(0, "添加好友成功", "", value));
                    else rsp.set_content(get_json(ret, "添加好友失败", "", value));
                }
            }
            else if(req_method == "GET")
            {
                rsp.set_content(get_json(-1, "请求方法暂不可用", "", value));
                rsp.set_status("400");
            }
        }
        else if(req_script == "/api/publish")
        {
            if(req_method == "POST")
            {
                //这里应该是查表单
                if(!doc.HasMember("user_id") || !doc.HasMember("content") 
                   || !doc["user_id"].IsInt() || !doc["content"].IsString())
                {
                    rsp.set_content(get_json(-1, "请求的信息错误", "", value));
                }
                //成功处理
                else
                {
                    int ret = -1;
                    int user_id = doc["user_id"].GetInt();
                    ret = user_mgr.CheckExist(user_id);
                    if(ret != USER_EXIST)
                    {
                        rsp.set_content(get_json(ret, "用户不存在", "", value));
                        goto RESPONSE;
                    }
                    char *send_buf = NULL, *recv_buf = NULL;
                    //发送消息，返回消息id
                    ssp::PublishMessageReq *publish_mess_req = req_base.mutable_publish_mess_req();
                    publish_mess_req -> set_user_id(user_id);
                    publish_mess_req -> set_content(doc["content"].GetString());
                    req_base.set_mess_type(PUBLISH_MESSAGE_REQ);
                    //从bus_mgr中获取sendbuf
                    int wait_cnt = 10;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_send_buf(3, send_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "send_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }
                    req_base.SerializeToArray(send_buf, 10239);
                    bus_mgr.fresh_next_send(3);
                    wait_cnt = 10, ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_recv_buf(2, recv_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "recv_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }

                    rsp_base.ParseFromArray(recv_buf, 10239);
                    ssp::PublishMessageRsp *publish_mess_rsp = rsp_base.mutable_publish_mess_rsp();
                    int message_id = publish_mess_rsp -> ret();
                    bus_mgr.fresh_next_recv(2);
                    if(message_id < 0)
                    {
                        rsp.set_content(get_json(message_id, "发送消息失败", "", value));
                        goto RESPONSE;
                    }

                    //获取好友列表
                    send_buf = NULL, recv_buf = NULL;
                    //req_base.clear_publish_mess_req();
                    //rsp_base.clear_publish_mess_rsp();
                    req_base.Clear();
                    rsp_base.Clear();
                    ssp::GetFriendsReq *get_friends_req = req_base.mutable_get_friends_req();
                    req_base.set_mess_type(GET_FRIENDS_REQ);
                    get_friends_req -> set_user_id(user_id);
                    log_mgr.println("mess_type : %d\n", req_base.mess_type());
                    wait_cnt = 5, ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_send_buf(1, send_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "send_buf:服务器忙", "", value));
                        log_mgr.println("关系服务器getsendbuf失败");
                        goto RESPONSE;
                    }
                    req_base.SerializeToArray(send_buf, 10239);
                    req_base.set_mess_type(0);
                    req_base.ParseFromArray(send_buf, 10239);
                    log_mgr.println("parse mess_type %d", req_base.mess_type());
                    log_mgr.println("ret_size = [%d]", req_base.ByteSizeLong());
                    bus_mgr.fresh_next_send(1);
                    wait_cnt = 5, ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_recv_buf(0, recv_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "recv_buf:服务器忙", "", value));
                        log_mgr.println("关系服务器getrecvbuf失败");
                        goto RESPONSE;
                    }
                    rsp_base.ParseFromArray(recv_buf, 10239);
                    bus_mgr.fresh_next_recv(0);
                    ssp::GetFriendsRsp *get_friends_rsp = rsp_base.mutable_get_friends_rsp();
                    ret = get_friends_rsp -> ret();
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "成功", "", value));
                        log_mgr.println("获取好友列表失败, ret = %d", ret);
                        goto RESPONSE;
                    }

                    //创建快照
                    send_buf = NULL, recv_buf = NULL;
                    //req_base.clear_get_friends_req();
                    req_base.Clear();
                    ssp::CreatePhotoReq *create_photo_req = req_base.mutable_create_photo_req();
                    for(int i = 0; i < get_friends_rsp -> friend_id_size(); i++)
                    {
                        create_photo_req -> add_user_id(get_friends_rsp -> friend_id(i));
                    }
                    create_photo_req -> set_message_id(message_id);
                    create_photo_req -> set_publisher_id(user_id);
                    create_photo_req -> set_time(now);
                    req_base.set_mess_type(CREATE_PHOTO_REQ);
                    wait_cnt = 5, ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_send_buf(5, send_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "send_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }
                    req_base.SerializeToArray(send_buf, 10239);
                    log_mgr.println("ret_size = [%d]\n", req_base.ByteSizeLong());
                    bus_mgr.fresh_next_send(5);
                    //rsp_base.clear_get_friends_rsp();
                    rsp_base.Clear();
                    wait_cnt = 10, ret = -1;
                    while(wait_cnt-- && ret != SUCCESS)
                    {
                        usleep(5000);
                        ret = bus_mgr.get_recv_buf(4, recv_buf);
                    }
                    if(ret != SUCCESS)
                    {
                        rsp.set_content(get_json(ret, "recv_buf:服务器忙", "", value));
                        goto RESPONSE;
                    }
                    rsp_base.ParseFromArray(recv_buf, 10239);
                    bus_mgr.fresh_next_recv(4);
                    ssp::CreatePhotoRsp *create_photo_rsp = rsp_base.mutable_create_photo_rsp();
                    ret = get_friends_rsp -> ret();
                    rsp.set_content(get_json(ret, "成功", "", value));
                    if(ret != SUCCESS)
                    {
                        log_mgr.println("创建快照失败");
                        goto RESPONSE;
                    }
                }
            }
            else if(req_method == "GET")
            {
                rsp.set_content(get_json(-1, "请求方法暂不可用", "", value));
                rsp.set_status("400");
            }
        }
        else
        {
            rsp.set_content(get_json(-1, "URL不合法", "", value));
            
        }
        RESPONSE:
        //回送消息
        cout << rsp.GetResponse();
        if (content) delete []content;
    }

#if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
    cin  = cin_streambuf;
    cout = cout_streambuf;
    cerr = cerr_streambuf;
#else
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
#endif

    return 0;
}
