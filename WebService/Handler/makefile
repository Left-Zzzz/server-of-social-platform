handle:all
all:mainhandler loginhandler relationhandler messagehandler
INC=-I../../include
mainhandler:./MainHandler.cpp 
	g++ -c -o ./lib/mainhanlder.o MainHandler.cpp $(INC) -g
loginhandler:./LoginHandler.cpp
	g++ -c -o ./lib/loginhandler.o LoginHandler.cpp $(INC) -g
relationhandler:./RelationHandler.cpp
	g++ -c -o ./lib/relationhandler.o RelationHandler.cpp $(INC) -g
messagehandler:./MessageHandler.cpp 
	g++ -c -o ./lib/messagehandler.o MessageHandler.cpp $(INC) -g
