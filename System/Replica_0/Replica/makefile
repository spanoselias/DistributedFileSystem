CC=gcc

make:  replica.c    
	gcc replica.c -lpthread  `pkg-config --cflags --libs glib-2.0`   -o repli

	 

clean:
	rm -f *.o cli rep  

