
CC= gcc -Wall
HEADER_FILES_DIR = #./headerfiles
INCLUDES = -I $(HEADER_FILES_DIR)
MAIN= ejecutable
SRCS = $(wildcard *.c)
DEPS = $(HEADER_FILES_DIR)/$(wildcard *.h)
OBJS = $(SRCS:.c=.o) 

$(MAIN): $(OBJS)
	$(CC) -o $(MAIN) $(OBJS) $(LIBS) 

%.o: %.c $(DEPS)
	$(CC) -c $< $(INCLUDES)

cleanall: clean
	rm -f $(MAIN)
clean:
	rm -f *.o *~
	
