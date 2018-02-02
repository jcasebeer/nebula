NAME = Nebula
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

INCLUDE = -IC:\SDL2\i686-w64-mingw32\include
LIB = -LC:\SDL2\i686-w64-mingw32\lib -lSDL2main -lSDL2 -lopengl32 -lglu32 -lglew32 -lopenal32 -llibsndfile-1 -lm
CFLAGS = -Wall -Wextra -std=c99 -O1 -msse2 -mfpmath=sse $(INCLUDE)
LFLAGS = -lmingw32 -mwindows -mconsole $(LIB)

all: $(NAME)
	./$(NAME)

$(NAME): $(OBJS)
	gcc $(OBJS) $(LFLAGS) -o $(NAME)

$(OBJS): $(SRCS)
	gcc $(CFLAGS) -c $(SRCS)

clean:
	rm -rf $(OBJS)
	rm -rf $(NAME).exe
