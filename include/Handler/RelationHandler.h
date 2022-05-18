/*************************************************************************
	> File Name: RelationHandler.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 27 Mar 2022 03:29:14 PM CST
 ************************************************************************/

#ifndef _RELATIONHANDLER_H
#define _RELATIONHANDLER_H

#include"MainHandler.h"
class RelationHandler:virtual public MainHandler
{
private:
    RelationHandler();
    ~RelationHandler();
    RelationHandler(RelationHandler&);
    RelationHandler operator=(RelationHandler&);
    static RelationHandler *instance_;
    const string SCRIPT_NAME_ = "/relation";
public:
    virtual int Handle(string &req_method, int user_id, string &input_data);
    virtual int Handle(string &req_method, int user_id, stringkv &input_data);
    virtual int Handle(string &req_method, int user_id, string &input_data_body, stringkv &input_data_query);
    static RelationHandler *GetInstance();
    int AddFriendHandle(string &req_method, int user_id, string &input_data_json, stringkv &input_data_query);
};
#endif
