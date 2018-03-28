gcc -Wall -Wextra -std=c11 -O1 -msse2 -mfpmath=sse -IC:\SDL2\i686-w64-mingw32\include -c build.c
gcc build.o -lmingw32 -mwindows -mconsole -LC:\SDL2\i686-w64-mingw32\lib -lSDL2main -lSDL2 -lopengl32 -lglu32 -lglew32 -lopenal32 -llibsndfile-1 -lm -o nebula.exe
./nebula.exe
