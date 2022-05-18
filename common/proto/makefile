proto:all
all:
	protoc --cpp_out=./ *.proto
	g++ -c user_info_base.pb.cc -o ../lib/proto/user_info_base.o
	g++ -c message_info_base.pb.cc -o ../lib/proto/message_info_base.o
	g++ -c photo_info_base.pb.cc -o ../lib/proto/photo_info_base.o
	g++ -c relation_info_base.pb.cc -o ../lib/proto/relation_info_base.o
	g++ -c message_define.pb.cc -o ../lib/proto/message_define.o
	mv *.h ../include/proto/
	rm *.cc
