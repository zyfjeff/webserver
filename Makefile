CFLAGS   	= -Wall -g
SOURCES 	= server.c libsocket.c module.c common.c main.c
OBJECTS		= $(SOURCES:.c=.o)
MODULES		= time.so issue.so diskfree.so process.so

.PHONY:		all clean
all: server $(MODULES)

clean:
	rm -rf $(OBJECTS) $(MODULES) server

server:	$(OBJECTS)
	$(CC) $(CFLAGS) -export-dynamic -o $@ $^ -ldl

$(OBJECTS):	common.h server.h module.h libsocket.h


$(MODULES):
%.so:	%.c common.c module.c common.h module.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<
