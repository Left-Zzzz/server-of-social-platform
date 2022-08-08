/*************************************************************************
	> File Name: TimeCommon.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Wed 18 May 2022 09:59:51 PM CST
 ************************************************************************/

#include"TimeCommon.h"

string TimeCommon::ConvertTimestamp(time_t timestamp, string &&time_format)
{
	/* 将time_t类型时间戳转换成指定格式时间字符串
	 * param: time_t timeestamp 时间戳
	 * param: string &&time_format 需要转换成的的时间文本格式，如"%a, %d %b %Y %H:%M:%S GMT"
	 * return: string, 格式转换后的文本
	**/
	struct tm *ptm = nullptr;
	ptm = gmtime(&timestamp);
	char time_str[80];
	strftime(time_str, 80, time_format.c_str(), ptm);
	return string(time_str);
}

#ifdef DEBUG
#include<iostream>
int main()
{
	time_t now = time(0);
	//Expires=Tue, 17 May 2022 17:03:34 GMT
	cout << TimeCommon::ConvertTimestamp(now, "%a, %d %b %Y %H:%M:%S GMT");
	return 0;
}
#endif