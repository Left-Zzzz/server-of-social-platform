/************************************************************************
	> File Name: AuthLib.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Wed 26 Jan 2022 04:07:33 PM CST
 ************************************************************************/
#include"Auth/AuthLib.h"

/*
 * SessionManager
 *
 * */

const string SessionManager::SESSION_KEY_PREFIX = "ssp:session:";
SessionManager::SessionManager()
{
}

SessionManager *SessionManager::GetInstance()
{
    if(m_Instace == NULL)
    {
        srand(time(0));
        m_Instace = new SessionManager();
    }
    return m_Instace;
}

/**
 * @brief 创建Session,将Session储存到Redis中，返回Sessionid
 * 
 * @param user_id string user_id:创建会话的user_id
 * @return string 返回的session_id
 */
string SessionManager::CreateSession(string user_id)
{
    string sessionid_;
    time_t now = time(0);
    //获取5位随机数
    int rand_num = rand() % 90000 + 10000;
    //sessionid：prefix+当前时间时间戳+五位随机数+user_id组合
    sessionid_ = prefix_ + to_string(now) + to_string(rand_num) + user_id;
    //保存Session到Redis中
    RedisManager *redis_mgr =  RedisManager::GetInstance();
    redis_mgr -> set(SESSION_KEY_PREFIX + sessionid_, user_id);
    redis_mgr -> expire(SESSION_KEY_PREFIX + sessionid_, to_string(expiration_sec));
    return sessionid_; 
}


/**
 * @brief 从Redis中验证Session
 * 
 * @param session_id 传入的待验证session_id
 * @return int 返回user_id
 */
int SessionManager::ValidateSession(string &session_id)
{
    LogManager *log_mgr = LogManager::GetInstance();
    RedisManager *redis_mgr = RedisManager::GetInstance();
    int get_status;
    string user_id = redis_mgr -> get(SESSION_KEY_PREFIX + session_id, get_status);
    log_mgr -> println("[SessionManager]:user_id=[%s]", user_id.c_str());
    if(user_id == "")
        return SESSION_NOT_EXIST;
    //更新Session有效期
    redis_mgr -> expire(SESSION_KEY_PREFIX + session_id, to_string(expiration_sec));
    return atoi(user_id.c_str());
}

SessionManager* SessionManager::m_Instace = NULL;


/*
 * TokenManager
 *
 * */

string TokenManager::hash_sha256(string &str) const
{
	char buf[3];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::string NewString;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(buf,"%02x",hash[i]);
        NewString = NewString + buf;
    }
	return NewString;
}

TokenManager::TokenManager()
{
}

TokenManager *TokenManager::GetInstance()
{
    if(m_Instace == NULL)
        m_Instace = new TokenManager;
    return m_Instace;
}

string TokenManager::CreateToken(string &sessionid, string &user_id, int now) const
{
    string expire = to_string(now + expiration_sec);
    return CreateToken(sessionid, user_id, expire);
}

string TokenManager::CreateToken(string &user_id, string &expire) const
{
    LogManager *log_mgr = LogManager::GetInstance();
    SessionManager *sess_mgr = SessionManager::GetInstance();
    string sessionid = sess_mgr -> CreateSession(user_id);
    return CreateToken(sessionid, user_id, expire);
}

string TokenManager::CreateToken(string &sessionid, string &user_id, string &expire) const
{
    string ret_token = salt + sessionid + user_id + expire + salt; //加盐
    string hash_sign = hash_sha256(ret_token);
    ret_token = sessionid + "|" + user_id + "|" + expire + "|" + hash_sign;
    return ret_token;
}

/**
 * @brief 解析Token
 * 
 * @param buf 传入的token
 * @param sessionid 返回的解析到的session_id
 * @param user_id 返回的解析到的user_id
 * @param expire 返回的解析到的expire
 * @param hash_old 返回的解析到的hash值
 * @return int 状态码，成功返回SUCCESS, 失败返回ERROR
 */
int TokenManager::ParseToken(char *buf, string &sessionid, string &user_id, string &expire, string &hash_old)
{
    sessionid = strtok(buf, "|");
    if(sessionid == "") return ERROR;
    user_id = strtok(NULL, "|");
    if(user_id == "") return ERROR;
    expire =  strtok(NULL, "|");
    if(expire == "") return ERROR;
    hash_old = strtok(NULL, "|");
    if(hash_old == "") return ERROR;
    return SUCCESS;
}


/**
 * @brief 验证token
 * 
 * @param token 待验证的oken
 * @return int 返回对应状态码
 */
int TokenManager::ValidateToken(string &token)
{
    LogManager *log_mgr = LogManager::GetInstance();
    log_mgr -> println("[TokenManager]解析Token");
    //解析Token
    string sessionid, user_id, expire, hash_old;
    char buf[1024];
    strcpy(buf, token.c_str());
    int ret = ParseToken(buf, sessionid, user_id, expire, hash_old);
    if(ret < 0) return ret;
    string hash_str = salt + sessionid + user_id + expire + salt;
    string hash_new_str = hash_sha256(hash_str);
    string hash_old_str = hash_old;
    //检验sha256值
    if(hash_new_str != hash_old_str)
        return TOKEN_INVALID;
    //验证Redis中是否存在该Session
    log_mgr -> println("[TokenManager]验证Session");
    SessionManager *session_mgr = SessionManager::GetInstance();
    log_mgr -> println("[TokenManager]sessionid=[%s]", sessionid.c_str());
    int user_id_redis = session_mgr -> ValidateSession(sessionid);
    if(user_id_redis < 0) return user_id_redis;
    //判断Redis中获取的Session信息是否与Token信息匹配
    if(user_id_redis != atoi(user_id.c_str())) return TOKEN_INVALID;
    //如果Token过期，更新Token
    if(atoi(expire.c_str()) < time(0))
    {
        log_mgr -> println("[TokenManager]Token过期，重新创建Token");
        ResponseManager *response_mgr = ResponseManager::GetInstance();
        string token_new = CreateToken(sessionid, user_id, time(0));
        response_mgr -> set_cookie("ssp_login", token_new);
    }
    log_mgr -> println("[TokenManager]解析Token结束");
    return atoi(user_id.c_str());
}


string TokenManager::CreateToken(string &user_id) const
{
    string expire = to_string(time(0) + expiration_sec);
    return CreateToken(user_id, expire);
}


/**
 * @brief 创建token并设置cookie
 * 
 * @param user_id 传入的要创建token的user_id
 * @return int 返回状态码
 */
int TokenManager::CreateTokenCookie(string &user_id) const
{
    time_t now = time(0);
    time_t expire = time(0) + expiration_sec;
    string expire_str = to_string(expire);
    string ret_token = CreateToken(user_id, expire_str);
    ResponseManager *response_mgr = ResponseManager::GetInstance();
    return response_mgr -> set_cookie(LOGIN_COOKIE_NAME, ret_token, expire);
}

TokenManager* TokenManager::m_Instace = NULL;

//正则表达式匹配
bool str_pattern(string &str, string &pattern)
{
    regex re(pattern);
    return regex_search(str, re);
}

