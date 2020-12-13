CC = gcc  
MAINC = main.c
EXEC = style
DEPLIB = -lXtst -lXmu -lXt -lXext -lX11
CFLAGS = `pkg-config --cflags --libs gtk+-3.0`
main: 
	$(CC) -g $(MAINC)  -o $(EXEC) $(CFLAGS) $(DEPLIB)
clean:
	rm $(EXEC) -rf
