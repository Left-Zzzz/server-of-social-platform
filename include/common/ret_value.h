/*************************************************************************
	> File Name: ret_value.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Fri 07 Jan 2022 10:21:29 PM CST
 ************************************************************************/

#ifndef _ERR_DEFINE_H
#define _ERR_DEFINE_H

#define OK 0
#define SUCCESS 0
#define ERROR -1

#define USER_EXIST -100
#define USER_NOT_EXIST -101
#define USER_NEED_DELETE -102
#define WRONG_PASSWORD -103
#define USER_TOO_MUCH -104
#define USER_UNAUTH -105
#define PERMISSION_DENINED -106

#define TYPE_NOT_DEFINE -500

#define IS_FRIEND -200
#define NOT_FRIEND -201
#define ALREADY_FRIEND -202
#define FRIEND_NEED_DELETE -203
#define FRIEND_TOO_MUCH -204
#define RELATION_NOT_FOUND -205

#define IS_BLACK -300
#define NOT_BLACK -301
#define ALREADY_BLACK -302
#define BLACK_NEED_DELETE -303
#define BLACK_TOO_MUCH -304

#define MESSAGE_NOT_EXIST -400

#define PHOTO_EXIST -500
#define PHOTO_NOT_EXIST -501

#define DB_CONN_INIT_FAIL -600
#define DB_CONN_CONNECT_FAIL -601
#define DB_QUERY_FAIL -602
#define DB_QUERY_EMPTY -603
#define REDIS_MUTEX_LOCKED -604
#define REDIS_COMMOND_FAILED -605

#define BUS_QUEUE_FULL -701
#define BUS_QUEUE_EMPTY -702
#define BUS_GET_HEAD_FAIL -703
#define BUS_GET_BUF_FAIL -704
#define BUS_CHECK_RECV_EMPTY -705
#define BUS_INVALID_CHANNEL_NAME -706

#define SCRIPT_NOT_EXIST -801
#define REQ_METHOD_NOT_EXIST -802
#define QUERY_MESSAGE_ERR -803

//Auth类状态码
#define VALIDATE_FAILED -900
#define SESSION_NOT_EXIST -901
#define TOKEN_INVALID -902
#define TOKEN_EXPIRE -903
#define COOKIE_EMPTY -904
#define COOKIE_KEY_NONE -905
#define COOKIE_VAL_NONE -906

#endif
