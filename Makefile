CC = gcc  
MAINC = main.c screen-style.c screen-stop.c -g screen-save.c
EXEC = style
CFLAGS = `pkg-config --cflags --libs gtk+-3.0`
main: 
	$(CC) -g $(MAINC)  -o $(EXEC) $(CFLAGS)
clean:
	rm $(EXEC) -rf
