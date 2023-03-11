SRCS	= filter.c m10.c manchester.c tsip.c checksum.c endian_util.c
OBJS	= $(SRCS:.c=.o)
CFLAGS	= -Wall
NAME	= libm10
CC		= gcc

ifeq ($(shell uname -s),Linux)
CFLAGS  += -fPIC
endif

ifeq ($(DEBUG), y)
CFLAGS	+= -g
endif

all:	$(OBJS)
	gcc -shared -o $(NAME).so $(OBJS)
	ar rcs -o $(NAME).a $(OBJS)

clean:
	rm -rf *.o *~ \#*
	rm -f $(NAME).so $(NAME).a

re:	clean all
