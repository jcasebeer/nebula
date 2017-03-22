NAME = Nebula
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

INCLUDE = -I/usr/local/include
LIB = -L/usr/local/lib -lSDL2 -lGLEW -lGLU -lGL -lm -Wl,-rpath=/usr/local/lib
CFLAGS = -Wall -Wextra -O3 -march=native -std=c99 $(INCLUDE)
LFLAGS = $(LIB)

all: $(NAME)
	./$(NAME)

$(NAME): $(OBJS)
	gcc $(OBJS) $(LFLAGS) -o $(NAME)

$(OBJS): $(SRCS)
	gcc $(CFLAGS) -c $(SRCS)

clean:
	rm -rf $(OBJS)
	rm -rf $(NAME)
