/*************************************************************************
	> File Name: MessageInfo.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 03:37:53 PM CST
 ************************************************************************/

#ifndef _MESSAGEINFO_H
#define _MESSAGEINFO_H
#include"common/macro.h"
#include"common/ret_value.h"
#include"proto/message_define.pb.h"
#include<string.h>
#include<AIGCJson.hpp>
using namespace std;
using namespace aigc;

struct MessageInfo
{
	GETSETVAR(int, message_id)
    GETSETVAR(int, publisher)
    GETSETVAR(std::string, content)
	GETSETVAR(int, publish_time)
    GETSETVAR(int, last_modify_time)
public:
    void FromPb(const ssp::MessageBase &pb_message);
    void ToPb(ssp::MessageBase *pb_message);
    AIGC_JSON_HELPER(message_id_, publisher_, content_, publish_time_, last_modify_time_);
    AIGC_JSON_HELPER_RENAME("message_id", "publisher", "content", "publish_time", "last_modify_time");
};

#endif
