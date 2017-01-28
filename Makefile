# Uncomment following line to activate Watchdog
ENABLE_WATCHDOG = y

SRCS	= main.c kml.c wgs84.c rotor.c config.c reader.c wordtab.c gps.c serial_unix.c
OBJS	= $(SRCS:.c=.o)
INC		= -I./m10
CFLAGS	= -Wall $(INC)
LDFLAGS	= -L./m10 -lm10 -lm
NAME	= trappette
CC		= gcc

ifeq ($(ENABLE_WATCHDOG), y)

SRCS	+= watchdog.c
CFLAGS	+= -DENABLE_WATCHDOG
LDFLAGS	+= -lrt -pthread

endif

##########

all: libm10 bin

bin: $(OBJS)
	$(CC) -static $(OBJS) -o $(NAME) $(LDFLAGS)

libm10:
	echo "### Building libm10 ###"; \
    (cd m10; make all);
	
clean:
	(cd m10; make clean);
	rm -rf *.o *~ \#*
	rm -f $(NAME)

fclean: clean

re:	clean all
