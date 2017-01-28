main.o: proj1.c
	gcc -Werror -o sshell proj1.c

clean:
	rm sshell
