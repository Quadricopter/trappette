SRCS	= main.c decoder.c kml.c wgs84.c rotor.c config.c reader.c wordtab.c gps.c serial_unix.c watchdog.c
OBJS	= $(SRCS:.c=.o)
INC		= -I../sdk
CFLAGS	= -Wall -Wpedantic -Wextra $(INC)
NAME	= trappette
CC		= gcc

LDFLAGS	+= -lm -ldl -lrt -pthread

ifeq ($(DEBUG), y)
CFLAGS	+= -g
endif

##########

all: $(OBJS)
	$(CC) -o ../$(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(NAME) *.o *~ \#*
