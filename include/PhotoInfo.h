/*************************************************************************
	> File Name: PhotoInfo.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 09 Jan 2022 03:50:08 PM CST
 ************************************************************************/

#ifndef _PHOTOINFO_H
#define _PHOTOINFO_H
#include"common/macro.h"
#include"common/ret_value.h"
#include"proto/photo_info_base.pb.h"
using namespace ssp;

struct PhotoInfo
{
public:
    void FromPb(ssp::PhotoInfoBase &photo);
    void ToPb(ssp::PhotoInfoBase &photo);
private:
    GETSETVAR(int, user_id)
    GETSETVAR(int, last_publisher)
    GETSETVAR(int, last_publish_time)
    GETSETVAR(int, last_publish_message_id)
};
#endif
