CFLAGS=-Wall -g
PROGS=helloworld

all: libcustom.a $(PROGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libcustom.a : lib.h fred.o bill.o
	ar -rs libcustom.a fred.o bill.o

$(PROGS): $(PROGS).c libcustom.a
	$(CC) $(CFLAGS) -o $(PROGS) $(PROGS).c -I. -L. -lcustom

clean:
	rm -f *.o
	rm -f $(PROGS)

install:
	cp $(PROGS) $(TARGET_DIR)/usr/bin
