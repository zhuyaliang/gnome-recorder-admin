CC = gcc  
MAINC = main.c screen-style.c screen-stop.c -g screen-save.c screen-count.c screen-window.c
EXEC = style
CFLAGS = `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1 libnotify`
main: 
	$(CC) -g $(MAINC)  -o $(EXEC) $(CFLAGS)
clean:
	rm $(EXEC) -rf
