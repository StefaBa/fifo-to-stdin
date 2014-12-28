
all: fi2sti

fi2sti: main.c
	gcc -lm -std=gnu99 -o fi2sti main.c

clean:
	rm -f fi2sti
