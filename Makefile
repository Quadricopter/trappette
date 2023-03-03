NAME	= trappette
CC		= gcc
LDFLAGS	= -lm -lrt -pthread

##########

all: libm10 libcore bin

bin:
	$(CC) -o $(NAME) core/core.a m10/libm10.a $(LDFLAGS)

libcore:
	echo "### Building core ###"; \
    (cd core; make all);
	
libm10:
	echo "### Building libm10 ###"; \
    (cd m10; make all);
	
clean:
	(cd m10; make clean);
	(cd core; make clean);
	rm -rf *.o *~ \#*
	rm -f $(NAME)

re:	clean all
