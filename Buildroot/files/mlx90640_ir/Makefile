DEPS = MLX90640_API.h 
OBJ = MLX90640_API.o
CFLAGS = -Wall -g -I. 
LIBS=-lm -lpthread
PROGS=mlx90640_ir

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
