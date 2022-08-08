#ifndef AUTHLIB_H
#define AUTHLIB_H
#include<bits/stdc++.h>
#include<openssl/sha.h>
#include<string.h>
#include"common/ret_value.h"
#include"RedisManager.h"
#include"ResponseManager.h"
#include"LogManager.h"
using namespace std;

class SessionManager
{
private:
    static SessionManager *m_Instace;
    static const int expiration_sec = 14400; //超时时间（s）
    static const string SESSION_KEY_PREFIX;
    const string prefix_ = "ssp_login_a";
    SessionManager();
    SessionManager(const SessionManager &);
    SessionManager &operator=(const SessionManager &);
public:
    static SessionManager *GetInstance();
    int ValidateSession(string &session_id);
public:
    string CreateSession(string user_id);
};

class TokenManager
{
private:
    static TokenManager *m_Instace; 
    static const int expiration_sec = 14400; //超时时间（s）
    string salt = "Left_ZzzzAsDfGhJkl%666!";  //盐值为固定值
    TokenManager();
    TokenManager(const TokenManager &) = delete;
    TokenManager &operator=(const TokenManager &) = delete;
    string hash_sha256(string &str) const;
    string CreateToken(string &user_id, string &expire) const;
    string CreateToken(string &sessionid, string &user_id, string &expire) const;
public:
    const string LOGIN_COOKIE_NAME = "ssp_login";
    static TokenManager *GetInstance();
    string CreateToken(string &sessionid, string &user_id, int now) const;
    string CreateToken(string &user_id) const;
    int CreateTokenCookie(string &user_id) const;
    //ret: SUCCESS 0, FAIL -1
    int ParseToken(char *buf, string &sessionid, string &user_id, string &expire, string &hash_old);
    //ret: SUCCESS 0, FAIL -1, EXPIRED -2
    int ValidateToken(string &token);
};
#endif
