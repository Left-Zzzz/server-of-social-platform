/*************************************************************************
	> File Name: MessageService.cpp
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sun 10 Apr 2022 02:19:21 PM CST
 ************************************************************************/

#include"Service/MessageService.h"

bool CheckIsDigit(string str)
{
    for(int i = 0; i < str.size(); ++i)
    {
        if(str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

