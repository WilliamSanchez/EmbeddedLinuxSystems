DEPS = matrices.h nav_func.h
OBJ = matrices.o nav_func.o
CFLAGS = -Wall -g -I. 
LIBS=-lm -lpthread
PROGS=uav_project

all: $(PROGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

$(PROGS): $(PROGS).o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o
	rm -f $(PROGS)

install:
	cp $(PROGS) $(TARGET_DIR)/usr/bin
