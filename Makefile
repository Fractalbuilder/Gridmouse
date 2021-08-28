Gridmouse:
	gcc `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs gtk+-3.0` -lxdo -o Gridmouse Gridmouse.c

clean:
	rm Gridmouse.c Sample.png