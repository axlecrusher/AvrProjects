CFLAGS:=-D_GNU_SOURCE -Wall -O2 -pg
INCLUDES:=

OBJ:= topside.o

LDFLAGS+=-lusb-1.0 -lpthread -pg -lrt  -I/opt/ssl/include/ -L/opt/ssl/lib/ -lcrypto 

all : test

%.o: %.c
	@rm -f $@
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wa,-ahl=$@.s -Wno-deprecated-declarations

%.o: %.cpp
	@rm -f $@
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wa,-ahl=$@.s -Wno-deprecated-declarations

test : $(OBJ)
	g++ -o $@ $^ $(LDFLAGS)

clean :
	rm -rf *.o  $~ test

