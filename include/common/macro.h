/*************************************************************************
	> File Name: macro.h
	> Author: Left
	> Mail: 1059051242@qq.com
	> Created Time: Sat 08 Jan 2022 11:26:22 PM CST
 ************************************************************************/

#ifndef _MACRO_H
#define _MACRO_H

#define myperror(str) \
{\
    perror(str);\
    exit(1);\
}

# define GETSETVAR(type, name) \
private:\
    type name##_;\
public:\
    const type& name() const\
    {\
        return name##_;\
    }\
    void set_##name(const type &val)\
    {\
        name##_ = val;\
    }

# define GETSETPTR(type,name) \
private:\
    type *name##_;\
public:\
    const type* name() const\
    {\
        return name##_;\
    }\
    void set_##name(const type* val)\
    {\
        name##_ = const_cast<type *>(val);\
    }

#define GETSETSTR(size, name) \
private:\
    char name##_[size];\
public:\
    const char* name() const\
    {\
        return name##_;\
    }\
    void set_##name(const char *val)\
    {\
        strcpy(name##_, val);\
    }
#endif
