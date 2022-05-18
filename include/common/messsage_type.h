/*************************************************************************
	> File Name: common/messsage_type.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 15 Jan 2022 02:53:04 PM CST
 ************************************************************************/

#ifndef _MESSSAGE_TYPE_H
#define _MESSSAGE_TYPE_H
#define LOGIN_REQ 101
#define LOGIN_RSP 102
#define LOGOUT_REQ 103
#define LOGOUT_RSP 104
#define DELETE_USER_REQ 105
#define DELETE_USER_RSP 106

#define PUBLISH_MESSAGE_REQ 201
#define PUBLISH_MESSAGE_RSP 202
#define GET_MESSAGE_REQ 203
#define GET_MESSAGE_RSP 204
#define CREATE_PHOTO_REQ 205
#define CREATE_PHOTO_RSP 206
#define GET_PHOTO_REQ 207
#define GET_PHOTO_RSP 208
#define DELETE_MESSAGE_REQ 209
#define DELETE_MESSAGE_RSP 210
#define MODIFY_MESSAGE_REQ 211
#define MODIFY_MESSAGE_RSP 212

#define ADD_FRIEND_REQ 301
#define ADD_FRIEND_RSP 302
#define DEL_FRIEND_REQ 303
#define DEL_FRIEND_RSP 304
#define ADD_BLACK_REQ 305
#define ADD_BLACK_RSP 306
#define DEL_BLACK_REQ 307
#define DEL_BLACK_RSP 308
#define GET_FRIENDS_REQ 309
#define GET_FRIENDS_RSP 310

#define SSP_REG_REQ 401
#define SSP_REG_RSP 402

#define BUS_UNDEFINED_TYPE 1
#define BUS_USER_TYPE 2
#define BUS_RELA_TYPE 3
#define BUS_MESS_TYPE 4
#define BUS_PHOTO_TYPE 5
#endif
