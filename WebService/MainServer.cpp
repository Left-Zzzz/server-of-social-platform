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
#include "AuthLib.h"
#include "RelationInfo.h"
#include "Handler/MainHandler.h"
extern char ** environ;
using namespace std;
using namespace rapidjson;

//初始化日志
void log_fp_init(LogManager *log_mgr, char *file_path)
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
    log_mgr -> log_fp_init(file_path_str.c_str());
    log_mgr -> println("svrno:%d\n", svrno);
}

//切割cookies
int cut_cookies(char *cookie, unordered_map<string, string> &map)
{
    LogManager *log_mgr = LogManager::GetInstance();
    log_mgr -> println("cookies: [%s]",cookie);
    int cnt = 0;
    vector<char *> kv_temp;
    char *p, *k, *v;
    p = strtok(cookie, ";");
    if(!p) return 0;
    kv_temp.push_back(p);
    while(p = strtok(NULL, ";"))
        kv_temp.push_back(p);
    for(auto p1 : kv_temp)
    {
        k = strtok(p1, "=");
        if(!k) continue;
        v = strtok(NULL, "\0");
        if(!v) continue;
        map[k] = v;
        log_mgr -> println("cut_cookies: name = [%s], value = [%s]", k, v);
        log_mgr -> println("instringkv: name = [%s], value = [%s]", k, map[k].c_str());
    }
    return cnt;
}

//url解码
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

//POST消息体大小最大值
static const unsigned long STDIN_MAX = 1000000;

//获取POST中的消息体数据
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

//生成resopnseJSON信息
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
        v = strtok(NULL, "\0");
        if(v) map[k] = v;
        else map[k] = "";
    }
}

int main (void)
{
    //需要用到的Manager直接放这里了
    LogManager *log_mgr = LogManager::GetInstance(); 
    log_fp_init(log_mgr,  (char *)"websvr.log");
    UserManager user_mgr;
    TokenManager *token_mgr = TokenManager::GetInstance();
    ResponseManager *rsp_mgr = NULL;

    //fcgi库的东西
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0)
    {
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
        if(rsp_mgr) delete rsp_mgr;
        rsp_mgr = ResponseManager::GetInstance();
        //储存POST方法表单解析后的数据
        unordered_map<string, string> form_data;
        //储存GET方法URI解析后的数据
        unordered_map<string, string> query_data;
        char *content;
        char content_decode[1024] = {0};
        
        //获取cookie并拆分
        //获取cookies原始数据
        char *cookie = FCGX_GetParam("HTTP_COOKIE", request.envp);
        //储存cookies的哈希表
        unordered_map<string, string> cookies_kv;
        int cookies_cnt;
        if(cookie) cookies_cnt = cut_cookies(cookie, cookies_kv);

        //获取POST中消息体的数据
        unsigned long clen = gstdin(&request, &content);
        //根据请求方式处理请求
        char *temp = FCGX_GetParam("REQUEST_METHOD", request.envp);
        string req_method;
        if(temp) req_method = temp;

        temp = FCGX_GetParam("SCRIPT_NAME", request.envp);
        string req_script;
        if(temp) req_script = temp;
        log_mgr -> println("req_script = [%s]", temp);
        //对请求作相应
        MainHandler *main_handler = MainHandler::GetInstance();
        //解析URI具体参数
        if(req_method == "GET")
        {
            char query_string[512], query_decode[512];
            temp = FCGX_GetParam("QUERY_STRING", request.envp);
            if(temp)
            {
                strcpy(query_string, temp);
                urldecode(query_string, query_decode);
                ParseUrl(query_decode, query_data);
            }
            //调用MainHandler处理信息
            main_handler -> Handle(req_method, cookies_kv, req_script, query_data);
        }
        //如果不是GET
        else
        {
            //conten:收到的内容，clen：收到内容大小
            string content_type;
            temp = FCGX_GetParam("CONTENT_TYPE", request.envp);    
            if(temp) content_type = temp;
            log_mgr -> println("%s", content);
            if(content_type == "application/x-www-form-urlencoded" 
               || content_type == "application/form-data")
            {
                urldecode(content, content_decode);
                ParseUrl(content_decode, form_data);
                //调用MainHandler处理信息
                main_handler -> Handle(req_method, cookies_kv, req_script, form_data);
            }
            else if(content_type == "application/json")
            {
                char query_string[512], query_decode[512];
                string json_data = content;
                //判断有无附加参数
                temp = FCGX_GetParam("QUERY_STRING", request.envp);
                if(temp)
                {
                    log_mgr -> println("query_string:%s", temp);
                    strcpy(query_string, temp);
                    urldecode(query_string, query_decode);
                    ParseUrl(query_decode, query_data);
                    for(auto &it : query_data)
                    {
                        log_mgr -> println("query_data:%s %s", it.first.c_str(), it.second.c_str());
                    }
                    main_handler -> Handle(req_method, cookies_kv, req_script, json_data, query_data);
                }
                else main_handler -> Handle(req_method, cookies_kv, req_script, json_data);
            }
            else
            {
                string raw_data = content;
                main_handler -> Handle(req_method, cookies_kv, req_script, raw_data);
            }
        }

        //回送消息
        cout << rsp_mgr -> GetResponse();
        if(content) delete []content;
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
