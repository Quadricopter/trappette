NAME	= trappette

all: libm10 libcore

libcore:
	echo "### Building core ###"; \
    (cd core; make all);
	
libm10:
	echo "### Building libm10 ###"; \
    (cd m10; make all);
	
clean:
	(cd m10; make clean);
	(cd core; make clean);
	rm -rf $(NAME) *.so *~ \#*
