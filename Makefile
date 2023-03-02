build/main: main.o app.o chip8.o safe_string.o 
	gcc -o build/main main.o app.o chip8.o safe_string.o -lmingw32 -lSDL2main -lSDL2

main.o: src/core/main.c
	gcc -c -DDEBUG -Iinclude src/core/main.c
app.o: src/core/app.c
	gcc -c -DDEBUG -Iinclude src/core/app.c
chip8.o: src/core/chip8.c
	gcc -c -DDEBUG -Iinclude src/core/chip8.c
safe_string.o: src/utils/safe_string.c
	gcc -c -DDEBUG -Iinclude src/utils/safe_string.c

clean:
	del *.o
	del build\main.exe