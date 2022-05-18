/*************************************************************************
	> File Name: TimeCommon.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Wed 18 May 2022 09:50:45 PM CST
 ************************************************************************/

#ifndef _TIMECOMMON_H
#define _TIMECOMMON_H
#include<time.h>
class TimeCommon
{
public:
    //将时间戳转换为指定格式字符串
    static string ConvertTimestamp(time_t timestamp, string &&time_format);
};
#endif
