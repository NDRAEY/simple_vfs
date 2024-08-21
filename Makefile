FILES = vfs.c debugfs.c main.c
OBJS = ${FILES:.c=.o}

all: $(OBJS)
	$(CC) $(OBJS) -g -o vfs

$(OBJS): %.o: %.c
	$(CC) $< -g -c -o $@

clean:
	@-rm $(OBJS)

files:
	@echo $(FILES)
	@echo $(OBJS)
