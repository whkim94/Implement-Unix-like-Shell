main.o: trythis.c
	gcc -Werror -o sshell trythis.c

clean:
	rm sshell