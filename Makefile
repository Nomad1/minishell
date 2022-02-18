tls:
	gcc -O0 -nostdlib server.c routines.c -fpic -o server
	objcopy -O binary --only-section=.text server server.bin
	xxd -i server.bin > server.h
clean:
	rm tls.bin tls
