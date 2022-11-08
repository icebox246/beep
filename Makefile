beep: beep.c
	gcc -o $@ $^ `pkg-config alsa --libs --cflags` -lm

