CC=g++
CFLAGS=-O3 -Wall -std=c++11 -I/usr/include/mysql -fPIC -I/usr/include/python2.6 -I/usr/include/boost
BOOSTLIB=/usr/lib64/boost
LDFLAGS=-shared -L/usr/lib64/mysql -L$(BOOSTLIB) -L/usr/local/lib -lboost_python -lpython2.6 -lmysqlclient -pthread
OBJECTS=MBDatabase.o MBConnection.o PyMain.o

mbdb.so: $(OBJECTS)
        $(CC) $(OBJECTS) -o mbdb.so $(LDFLAGS)

%.o: %.cpp
        $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
        rm -f *.o
