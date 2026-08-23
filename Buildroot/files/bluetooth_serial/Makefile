CFLAGS=-Wall -g
PROGS=bluetooth_serial

all: $(PROGS)

$(PROGS): $(PROGS).c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f $(PROGS)

install:
	cp $(PROGS) $(TARGET_DIR)/usr/bin
