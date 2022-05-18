#!/bin/bash
#启动MainServer
if [[ -e "./WebService/MainServer" ]];
then
    echo "MainServer启动中"
    cd WebService/ 
    cp MainServer subSvr/
    cd subSvr/
    spawn-fcgi -f ./MainServer -p 11235 -a 127.0.0.1 -F 1
    if [[ $? == 0 ]];
    then
        echo "MainServer启动成功"
    else
        echo "MainServer启动失败，错误码$?"
    fi
    cd ../../ 
else
    echo "MainServer不存在，启动失败。"
fi

#启动RelationSvr
if [[ -e "./Relation/RelationSvr" ]];
then
    echo "RelationSvr启动中"
    cd Relation/
    cp RelationSvr subSvr/
    cd subSvr/
    ./RelationSvr &
    if [[ $? == 0 ]];
    then
        echo "RelationSvr启动成功"
    else
        echo "RelationSvr启动失败，错误码$?"
    fi
    cd ../../ 

else
    echo "RelationSvr不存在，启动失败。"
fi

#启动MessageSvr
if [[ -e "./Message/MessageSvr" ]];
then
    echo "MessageSvr启动中"
    cd Message/ 
    cp MessageSvr subSvr/
    cd subSvr/
    ./MessageSvr &
    if [[ $? == 0 ]];
    then
        echo "MessageSvr启动成功"
    else
        echo "MessageSvr启动失败，错误码$?"
    fi
    cd ../../ 
else
    echo "MessageSvr不存在，启动失败。"
fi

#启动PhotoSvr
if [[ -e "./Photo/PhotoSvr" ]];
then
    echo "PhotoSvr启动中"
    cd Photo/
    cp PhotoSvr subSvr/
    cd subSvr/
    ./PhotoSvr &
    if [[ $? == 0 ]];
    then
        echo "PhotoSvr启动成功"
    else
        echo "PhotoSvr启动失败，错误码$?"
    fi
    cd ../../ 
else
    echo "PhotoSvr不存在，启动失败。"
fi
