server:
	gcc -Wno-unused-result -m64 -O1 -s -nostdlib -fPIC server.c routines.c commands.c -o server -D__COMPACT -D__LIBC
	objcopy -O binary --only-section=.text server server.bin
	xxd -i server.bin > server.h
	gcc -O0 -m64 test.c -o test
clean:
	rm server.bin server test
