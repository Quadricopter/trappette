SRCS	= libm10.c filter.c m10.c manchester.c tsip.c checksum.c endian_util.c
OBJS	= $(SRCS:.c=.o)
INC     = -I. -I../sdk
CFLAGS	= -Wall $(INC)
NAME	= libm10
CC		= gcc

ifeq ($(shell uname -s),Linux)
CFLAGS  += -fPIC
endif

ifeq ($(DEBUG), y)
CFLAGS	+= -g
endif

all:	$(OBJS)
	gcc -shared -o ../$(NAME).so $(OBJS)

clean:
	rm -rf *.o *~ \#*
	rm -f $(NAME).so

re:	clean all
