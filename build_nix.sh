gcc -Wall -Wextra -std=c11 -O3 -c build.c
gcc build.o -lSDL2main -lSDL2 -lGL -lGLEW -lopenal -lsndfile -lm -o nebula.exe
./nebula.exe
