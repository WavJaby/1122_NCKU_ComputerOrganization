CC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -static
TARGET := 1 2 3 4
v := 0

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^
1: 1.c
	$(CC) $(CFLAGS) -o $@ $^
2: 2.c
	$(CC) $(CFLAGS) -o $@ $^
3: 3.c
	$(CC) $(CFLAGS) -o $@ $^
4: 4.c
	$(CC) $(CFLAGS) -o $@ $^

	
judge: all
	-judge -c judge1.conf  
	@printf "or \`judge -c judge1.conf -v 1\n\`"
	-judge -c judge2.conf
	@printf "or \`judge -c judge2.conf -v 1\n\`"
	-judge -c judge3.conf
	@printf "or \`judge -c judge3.conf -v 1\n\`"
	-judge -c judge4.conf
	@printf "or \`judge -c judge4.conf -v 1\n\`"

clean:
	rm -f $(TARGET)
