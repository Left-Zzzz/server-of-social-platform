.SSP:all

MYSQL_LIB=-L/usr/lib/mysql -lmysqlclient
PROTO_LIB=`pkg-config --cflags --libs protobuf`
INC=-I./include
USER_CC=./lib/user.o ./lib/userinfo.o 
MESSAGE_CC=./lib/message_info.o
DB_CC=./lib/db.o
LOG_CC=./lib/log.o
CRYPTOLIB=-lcrypto
REDISLIB=-lhiredis
BUS_CC=./lib/bus.o
PROTO_CC=./lib/proto/*.o
RSP_CC=./lib/rsp.o
HANDLERS_CC=$(MAINHANDLER_CC) $(LOGINHANDLER_CC) $(RELATIONHANDLER_CC) $(MESSAGEHANDLER_CC)
MAINHANDLER_CC=./lib/mainhanlder.o 
LOGINHANDLER_CC=./lib/loginhandler.o
RELATIONHANDLER_CC=./lib/relationhandler.o
MESSAGEHANDLER_CC=./lib/messagehandler.o
SERVICES_CC=./lib/messageservice.o
AUTH_CC=./lib/authlib.o
REDIS_CC=./lib/redis.o
TIME_COMMON_CC=./lib/timecommon.o

all:redis auth handler response token bus log db user relation message photo server

timecommon:common/TimeCommon.cpp 
	g++ -c -o lib/timecommon.o ./common/TimeCommon.cpp $(INC) -g


redis:Db/RedisManager.cpp
	g++ -c -o lib/redis.o ./Db/RedisManager.cpp $(INC) -g

authlib:Auth/AuthLib.cpp
	g++ -c -o lib/authlib.o ./Auth/AuthLib.cpp $(INC) -g

log:LogManager.cpp
	g++ -c -o lib/log.o LogManager.cpp $(INC) -g

handler:WebService/MainHandler.cpp
	g++ -c -o lib/handler.o ./WebService/MainHandler.cpp ./WebService/LoginHandler.cpp $(INC)

response:WebService/ResponseManager.cpp
	g++ -c -o lib/rsp.o ./WebService/ResponseManager.cpp $(INC) -g

webservice:user 
	g++ -g ./WebService/MainServer.cpp $(SERVICES_CC) $(HANDLERS_CC) $(AUTH_CC) $(RSP_CC) $(HANDLER_CC) $(TIME_COMMON_CC) $(BUS_CC) $(USER_CC) $(MESSAGE_CC) $(DB_CC) $(LOG_CC) $(PROTO_CC) $(REDIS_CC) ./lib/libfcgi/*.o -o ./WebService/MainServer $(INC) $(MYSQL_LIB) $(PROTO_LIB) $(CRYPTOLIB) $(REDISLIB)

webservicetest:user webservice_test.cpp
	g++ -g webservice_test.cpp ResponseManager.cpp $(BUS_CC) $(USER_CC) $(DB_CC) $(LOG_CC) $(PROTO_CC) ./lib/libfcgi/*.o -o webservicetest $(INC) $(MYSQL_LIB) $(PROTO_LIB) $(CRYPTOLIB)

test:test.cpp
	g++ -o $@  lib/*.o test.cpp -I/usr/local/include/google/protobuf -L/usr/local/lib/libprotobuf.a -lprotobuf -g

server:main.cpp
	g++ -c $^ -o lib/main.o $(INC)
	g++ -g -o $@ lib/*.o -L/usr/lib/mysql -lmysqlclient `pkg-config --cflags --libs protobuf` -lpthread -g $(INC)

client:client.cpp
	g++ client.cpp -o client lib/*.o `pkg-config --cflags --libs protobuf` 

user:User/UserManager.cpp User/UserInfo.cpp
	g++ -c User/UserInfo.cpp -o lib/userinfo.o $(INC)
	g++ -c User/UserManager.cpp -o lib/user.o $(INC)

relation:log db bus Relation/RelationManager.cpp
	g++ -c Relation/RelationInfo.cpp -o lib/relation_info.o $(INC)
	g++ -c Relation/RelationManager.cpp -o lib/relation.o $(INC)
	g++ -o Relation/RelationSvr Relation/RelationServer.cpp lib/relation_info.o lib/relation.o $(PROTO_CC) $(PROTO_LIB) $(DB_CC) $(BUS_CC) $(LOG_CC) $(MYSQL_LIB) $(INC) -g

message:log db bus Message/MessageManager.cpp
	g++ -c Message/MessageManager.cpp -o lib/message.o $(INC) -g
	g++ -c Message/MessageInfo.cpp -o lib/message_info.o $(INC) -g
	g++ -o Message/MessageSvr Message/MessageServer.cpp lib/message.o lib/message_info.o $(PROTO_CC) $(PROTO_LIB) $(DB_CC) $(BUS_CC) $(LOG_CC) $(MYSQL_LIB) $(INC) -g

photo:Photo/PhotoManager.cpp
	g++ -c Photo/PhotoManager.cpp -o lib/photo.o $(INC)
	g++ -o Photo/PhotoSvr Photo/PhotoServer.cpp lib/photo.o $(PROTO_CC) $(PROTO_LIB) $(DB_CC) $(BUS_CC) $(LOG_CC) $(MYSQL_LIB) $(INC) -g

db:Db/DbManager.cpp
	g++ -c Db/DbManager.cpp -o lib/db.o -L/usr/lib/mysql -lmysqlclient $(INC)

bus:Bus/BusManager.cpp
	g++ -c Bus/BusManager.cpp -o lib/bus.o $(INC) -g

.SSP:clean
clean:
	rm -f server *.o
