# Uncomment following line to activate Watchdog
#ENABLE_WATCHDOG = y
include ../config.mk

SRCS	= main.c kml.c wgs84.c rotor.c config.c reader.c wordtab.c gps.c serial_unix.c
OBJS	= $(SRCS:.c=.o)
INC		= -I../m10
CFLAGS	= -Wall $(INC)
NAME	= core
CC		= gcc

ifeq ($(ENABLE_WATCHDOG), y)

SRCS	+= watchdog.c
CFLAGS	+= -DENABLE_WATCHDOG
LDFLAGS	+= -lrt -pthread

endif

##########

all: $(OBJS)
	ar rcs -o $(NAME).a $(OBJS)

clean:
	rm -rf *.o *~ \#*

fclean: clean
	rm -f $(NAME).a

re:	fclean all
