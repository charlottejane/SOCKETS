listen: listener.c
	gcc -o listen listener.c -I.
talk: talker.c
	gcc -o talk talker.c -I.

clean:
	rm talk listen
	