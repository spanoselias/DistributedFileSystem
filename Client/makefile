CC=gcc

make:  client.c    
	gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
	 
	 

clean:
	rm -f *.o cli rep  

