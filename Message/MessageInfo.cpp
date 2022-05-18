/*************************************************************************
	> File Name: MessageInfo.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 10 Apr 2022 02:58:50 PM CST
 ************************************************************************/

#include"MessageInfo.h"

void MessageInfo::FromPb(const ssp::MessageBase &pb_message)
{
    set_message_id(pb_message.message_id());
    set_publisher(pb_message.publisher());
    set_content(pb_message.content());
    set_publish_time(pb_message.publish_time());
    set_last_modify_time(pb_message.last_modify_time());
}

void MessageInfo::ToPb(ssp::MessageBase *pb_message)
{
    pb_message -> set_message_id(message_id());
    pb_message -> set_publisher(publisher());
    pb_message -> set_content(content().c_str());
    pb_message -> set_publish_time(publish_time());
    pb_message -> set_last_modify_time(last_modify_time());
}
