CC=gcc

make:  filemanager.c	 
	gcc filemanager.c  `pkg-config --cflags --libs glib-2.0`  -lpthread -o fil
	 

clean:
	rm -f *.o cli fil  

