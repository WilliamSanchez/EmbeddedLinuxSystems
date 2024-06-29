CFLAGS=-Wall -g
PROGS=user_gpio-irq_interface

all: $(PROGS)

$(PROGS): $(PROGS).c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f $(PROGS)

install:
	cp $(PROGS) $(TARGET_DIR)/usr/bin
