SRCS	= main.c kml.c wgs84.c rotor.c config.c reader.c wordtab.c gps.c serial_unix.c watchdog.c
OBJS	= $(SRCS:.c=.o)
INC		= -I../m10
CFLAGS	= -Wall $(INC)
NAME	= core
CC		= gcc

LDFLAGS	+= -lrt -pthread  # Watchdog

ifeq ($(DEBUG), y)
CFLAGS	+= -g
endif

##########

all: $(OBJS)
	ar rcs -o $(NAME).a $(OBJS)

clean:
	rm -rf $(NAME).a *.o *~ \#*
	rm -f $(NAME).a

re:	clean all
