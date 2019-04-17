MYSQL_CONCPP_DIR = connector
CPPFLAGS =  -I $(MYSQL_CONCPP_DIR)/include
LDLIBS = -L/usr/bin -L/usr/local/bin -L $(MYSQL_CONCPP_DIR)/lib64 $(MYSQL_CONCPP_DIR)/lib64/*.so -lssl -lcrypto -lpthread -l:libmysqlcppconn8.so.1 -l:libcrypto.so.1.0.0
LINK.o = $(LINK.cc) # use C++ linker
CXXFLAGS = -std=c++11
db_test : db_test.cpp database_interface.cpp