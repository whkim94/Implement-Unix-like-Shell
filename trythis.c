#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

int exit_flag = 0;
/* Read one char from the keyboard */
static inline char get_one_char(void)
{
	int res = 0;
	char c;

	while (res <= 0) {
		res = read(STDIN_FILENO, &c, 1);

		if (res < 0) {
			if (errno == EINTR)
				/* read() was interrupted, try again */
				continue;
			/* Otherwise, it's a failure */
			return -1;
		}
	}
	return c;
}

/*
 * Non-canonical mode management
 */
static struct termios saved_termios;
static pid_t shell_pid;

/* Reset the terminal to the saved parameters */
static void termios_reset(void)
{
	tcsetattr(STDOUT_FILENO, TCSANOW, &saved_termios);
}

/* Reset the terminal to the saved parameters (for signals) */
static void termios_reset_sig(int signum)
{
	if (getpid() == shell_pid)
		termios_reset();
	exit(1);
}

/* Set the terminal in non-canonical mode */
static void termios_set_noncanonical(void)
{
	struct termios attr;
	struct sigaction act;
	int fd = STDOUT_FILENO;
	int err = 0;

	/* Nothing to do if we're not in a terminal */
	if (!isatty(fd))
		return;

	/* Save current attributes */
	tcgetattr(fd, &saved_termios);

	/* Set new "raw" attributes */
	tcgetattr(fd, &attr);
	attr.c_lflag &= ~ICANON;	/* Disable canonical mode */
	attr.c_lflag &= ~ECHO;		/* Disable character echoing */
	attr.c_cc[VMIN] = 1;		/* Read at least one character */
	attr.c_cc[VTIME] = 0;		/* No timeout */
	tcsetattr(fd, TCSAFLUSH, &attr);

	/* Make sure that we restore the previous mode upon exit */
	shell_pid = getpid();
	act.sa_handler = termios_reset_sig;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	err += sigaction(SIGINT, &act, NULL);
	err += sigaction(SIGHUP, &act, NULL);
	err += sigaction(SIGTERM, &act, NULL);
	err += sigaction(SIGQUIT, &act, NULL);
	if (err) {
		perror("sigaction");
		exit(1);
	}
}

void execute_command(char* cmd, int input_count) {
	//parse the command
/*	
	char str[] ="- This, a sample string.";
	char * pch;
	printf ("Splitting string \"%s\" into tokens:\n",str);
        pch = strtok (str," ,.-");

	while (pch != NULL)
	{
	        printf ("%s\n",pch);
	        pch = strtok (NULL, " ,.-");
	}
*/
	char *head;

	head = strtok(cmd," ");
	if ( strcmp(head,"exit") == 0) {
		write(1,"Bye...\n",7);
		exit_flag = 1;
		return;
	}
	else if( strcmp(head,"cd") == 0) {
		write(1,"start of cd\n",12);
		// char* current_dir = get_current_dir_name();
		// char* token = strtok(NULL,"\0");
		// //write(1,token,sizeof(token));
		// //char* new_dir = current_dir + token;
		// char* new_dir = strcat(current_dir,token);
		// write(1,new_dir,sizeof(new_dir));
		// chdir(new_dir);
		write(1,"finished cd\n",12);
	}
	else if( strcmp(head,"pwd") == 0) {
		char* dir_name = get_current_dir_name();
		//char* dir_name = getcwd();
		write(1,"current directory: ",strlen("current directory: "));
		write(1, dir_name, strlen(dir_name));
		write(1,"\n",1);
	}

	while(head != NULL){
		printf("%s\n", head);

		head = strtok(NULL," ");
	}

//	write(STDOUT_FILENO, "Parsed\n", 8);
//	printf("First: %s\n",head);

//	printf("Rest: %s\n",tail);
	
	
}

int main(int argc, char **argv)
{
	char c;
	char arr[512];
	int i = 0, flag = 0;
	int input_char = 0;
	/* Switch to non-canonical terminal mode */
	termios_set_noncanonical();

	/* Shell main loop */
	write(STDOUT_FILENO, "sshell$ ", 8);
	while (1) {
		c = get_one_char();
		
		if (c == 0x04){
	       // Ctrl - D
		   write(2,"Bye...\n",7);
	       break;
		}
		else if(isprint(c)) //the character is printable
		{
			arr[input_char] = c;
			input_char++;
			write(STDOUT_FILENO, &c, 1);
			//c = get_one_char();
		}	
	
		//the user presses enter	
		//parse_input(arr);
		else if (c == 0x0A) { //enter
			//printf("\nYou have pressed enter! asshole\n");
			printf("The current input is: %s\n",arr);
			execute_command(arr,input_char);
			if (exit_flag == 1) {
				return 0;
			}
			input_char = 0;
			memset(&arr, 0, sizeof(arr));
			write(STDOUT_FILENO, "sshell$ ", 8);
		}

		else if (c == 0x7F) { //backspace
			write(STDOUT_FILENO, &c, 1);
			//memset(&arr[input_char-1 ], 0, sizeof(arr));
			memset(&arr[input_char-1 ], 0, 1);

			input_char--;
			
		}

		if (exit_flag == 1) {
			break;
		}



	}

			

	/* Switch back to previous terminal mode */
	termios_reset();

	return EXIT_SUCCESS;
}


