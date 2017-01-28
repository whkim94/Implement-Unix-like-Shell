#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int exit_flag = 0;
char completeflag = '0';
extern char **environ;
char history[10][512]; //last 10 history of commands
int history_count = 0; //counts how many inputs we have typed
int history_current = 0; //the current history command we are on

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

void execute_command(char* cmd, int input_count);


void pipeline(char *process)
{
	
	int pd[2];
	pid_t pid;
	int fd_in = 0;
	int flag_pipe = 0;
	int numwords = 0; // result
	char *tempprocess = NULL;
	
	
    // state:
   tempprocess = process;
    int counter = 0;
// To count number of words from: http://stackoverflow.com/questions/12698836/counting-words-in-a-string-c-programming
    do switch(*tempprocess) {
        case '\0': 
					numwords++;
					break;
        case '|':
					if (counter) 
					{ 	counter = 0; 
						numwords++; 
					}
					break;
        default: 
					counter = 1;
    } while(*tempprocess++);

   // printf("Number of words = %d\n\n", numwords);
	//write(STDOUT_FILENO, "Here1", 5);
	char *head1[10] = {NULL};
	char *head2[10] = {NULL};
	char* proc1 = strtok(process, "|");
	char* proc2 ;
	char *tempproc = NULL;
	int k = numwords;
	int j = 0, i = 0;

	//for(j = 1; j<=numwords;j++){
	do{
		if (flag_pipe == 1)
			tempproc = proc2;
		proc2 = strtok(NULL, "|\0");
		if(flag_pipe == 1)
			proc1 = tempproc;
		i = 0;
		if (strchr(proc1,32)) {
			char* tokens1 = strtok(proc1," \0");
			while (tokens1 != NULL) {
				head1[i] = tokens1;
				tokens1 = strtok(NULL," |\0");
				i++;
			}
		}
		else{
			head1[0] = proc1;
		}
		pipe(pd);
		pid = fork();
		if (pid < 0){
			  exit(EXIT_FAILURE);
		}
		else if (pid == 0){
				close(0);
				dup(fd_in);
				if(j < k-1){
					close(pd[1]);
					dup(1);
					//dup2(pd[1], 1);
				}
				if(j<k)
				close(pd[0]);
				execvp(head1[0], head1);
				exit(EXIT_FAILURE);
		}
		else{
				wait(NULL);
				close(pd[1]);
				fd_in = pd[0]; 	
		}
			j++;
			flag_pipe = 1;
		}while(j != k);
	

}


void piping_test(char* cmd, int input_count) {
	int pcount=1; //already one pipe exist
	char *head=NULL, *tail=NULL, *head_p=NULL, *tail_p=NULL;
	char *arg1[100]={NULL}; //front process
	char *arg2[100]={NULL}; //back process
	int ofd=0, fd[2];
	
	pid_t pid;
	int i = 0;

 	head = strtok(cmd,"|");
 	tail = strtok(NULL,"|");
 	//tail = strtok(NULL,"|");

while(tail != NULL) {
 	head_p = strtok(head," ");

 	while(head_p != NULL){
 		arg1[i] = head_p;
 		head_p = strtok(NULL," ");
 		i++;
 	}

	while(head != NULL) {


		pipe(fd);
		
		pid=fork();

		if(pid == -1) {
			perror("fork");
			completeflag = '1';
			return;
		} 

		else if(pid == 0) {
			//close(fd[0]); //close unused reading in child
			//dup2(ofd, 0);
			//execute_command();
			//tail = strtok(NULL,"|");
			//if(tail != NULL)
			close(fd[0]);
			dup2(fd[1],1);
				//close(fd[1]);
			execute_command(*arg1, input_count);
			//exit(0);
			//execvp(arg1[0],arg1);
			exit(EXIT_FAILURE);
		}

		else {
			wait(NULL);
			//dup2(fd[1],1);
			close(fd[1]);
			//read(fd[0], arg1, strlen(arg1));
			tail = strtok(NULL,"|"); //check if there is after-parent process to run
			if(tail != NULL) {
				memset(arg1, 0, sizeof(*arg1)); // reset's arguments

				head = tail; // process 2 becomes child
			}
			else {
				int j=0; // counter
				char *fuu=NULL;
 				while(tail_p != NULL){
 					arg2[j] = tail_p;
 					tail_p = strtok(NULL," ");
 					j++;
 				}
 				//fuu = fd[1];
 				//arg2[j] = fuu;

 				execvp(arg2[0],arg2);
			}

		}

	}
}

}

void redirection(char* cmd, char *red1, char *red2) {

	int fd;
	pid_t pid;

	

	if(red1!=NULL) { // '>'
		int i = 0;
		char *redh=NULL, *redt=NULL, *redh_p=NULL;
		char *head[10]={NULL};

		redh = strtok(cmd,">"); //gets args before >
		redt = strtok(NULL," >"); //gets file name after >. Handles both '>' and ' '

		redh_p = strtok(redh," "); //separates arguments into command and sub-command ex) ls -al

		while(redh_p!=NULL) {
			head[i] = redh_p;
			redh_p = strtok(NULL," ");
			i++;
		}

		pid = fork();

		if(pid == -1) {
			perror("fork");
			completeflag = '1';
			return;
		}

		else if(pid == 0) {
			fd=open(redt, O_CREAT | O_RDWR, 0644);
			if(fd == -1) {
				write(STDOUT_FILENO, "Error: no output file\n", 22);
				completeflag = '1';
				return;
			}

			dup2(fd,1);
			close(fd);
			execvp(head[0],head);
		}

		else wait(NULL);
	}

	else { // '<'
		int i = 0;
		char *redh=NULL, *redt=NULL, *redh_p=NULL;
		char *head[10]={NULL};

		redh = strtok(cmd,"<"); //gets args before <
		redt = strtok(NULL,"< "); //gets file name after <. Handles both '<' and ' '

		redh_p = strtok(redh," "); //separates arguments into command and sub-command ex) ls -al

		while(redh_p!=NULL) {
			head[i] = redh_p;
			redh_p = strtok(NULL," ");
			i++;
		}

		pid = fork();

		if(pid == -1) {
			perror("fork");
			completeflag = '1';
			return;
		}

		else if(pid == 0) { //child
			fd=open(redt, O_RDONLY, 0644);
			if(fd == -1) {
				perror(redt);
				completeflag = '1';
				return;
			}

			head[i] = redt; //puts file name after sequence of arguments in the array

			//dup2(fd,1);
			close(fd);
			execvp(head[0],head);
		}

		else wait(NULL);

	}

}

void builtin_command(char *token, int input_count);

void execute_command(char* cmd, int input_count) {
	//parse the command

	char *head, *head2[10], *red1=NULL, *red2=NULL;
	int fd;
	int i = 0;
	char *pch = NULL;

	pch = strchr(cmd,124); //check if "|" is included

	if(pch != NULL){
		//piping_test(cmd, input_count);
		return;
	} //if there is piping


	red1 = strchr(cmd,62); //check if ">" is included
	red2 = strchr(cmd,60); //check if "<" is included

	//printf("number: %d\n",red1-cmd+1);
	//write(STDOUT_FILENO, red1-cmd+1, 8);	

	if(red1!=NULL || red2!=NULL){
		

		redirection(cmd,red1,red2);
		return;
	} //if there is redirection
		
	
	
	head = strtok(cmd," ");

	if(strcmp(head,"exit")==0 || strcmp(head,"pwd")==0 || strcmp(head,"cd")==0)
		builtin_command(head, input_count); //if builtin command for head


	else {

		int i = 1;
		char test;
		int pfd[2]; //pipe to read child exit failure
		char *letter[100] ={NULL};
		letter[0] = head;
		
		while(head != NULL){
			head = strtok(NULL," ");
			letter[i] = head;
			i++;
		}
		
		letter[i+1] = NULL;
		pid_t pid;
		int status;

		if(pipe(pfd) == -1) {
			perror("pipe in exec");
			exit(EXIT_FAILURE);
		}
		
		pid = fork();

		if(pid == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
			completeflag = '2';
			//exit(1);
		}
		
		else if(pid == 0){
			close(pfd[0]); //close unused reading pipe
			execvp(letter[0], letter);
			write(pfd[1], "1", 1);
			perror("execvp");
			exit(EXIT_FAILURE);
			completeflag = '5';
			//exit(1);
		} //child process
		
		else if(pid > 0){
			//waitpid(-1, &status, 0);
			close(pfd[1]); //close unused writing pipe
			wait(NULL);
			read(pfd[0], &test, 1);
			//write(STDOUT_FILENO, &test, 1);
			if(test == '1') {
				completeflag = '1';
				test = '0';
			} // if there is invalid command such as 'd', it should exit with error
			else
				completeflag = '0'; //no error
			close(pfd[0]);
			//exit(status);
		} 
		
		return;
	}
}

void builtin_command(char *token, int input_count) {
	if(strcmp(token,"exit") == 0){
        	exit_flag = 1;
        	write(2,"Bye...\n",7);
			return;
	} //it exit typed

	else if(strcmp(token,"cd") == 0){
		char* current_dir = get_current_dir_name();
		char* token = strtok(NULL,"\0");
		//write(2,"\n",1);
		if ( input_count == 2) { //user types only cd, then we go to home directory
			char* username = getlogin();
			//write(1,username,strlen(username));
			//write(1,"\n",1);
			const char* start_dest = "/home/";
			int size = sizeof(username)/sizeof(username[0]);
			char* home_dest = (char *) malloc(6+size);
			strcpy(home_dest,start_dest);
			strcat(home_dest,username);
			//write(1,home_dest,strlen(home_dest));
			//write(1,"\n",1);
			chdir(home_dest);
		}
		else { //we go to the directory typed by user
			char* new_dir1 = strcat(current_dir,"/");
			char* new_dir2 = strcat(new_dir1,token);
			//write(1,new_dir2,strlen(new_dir2));
			chdir(new_dir2);
			//write(1,"finished cd\n",12);
		}
	}//takes care of all cd commands

	else if(strcmp(token,"pwd") == 0){
		char* dir_name = get_current_dir_name();
		//write(2,"\n",1);
		write(1, dir_name, strlen(dir_name));
		write(2,"\n",1);
	}//gets directory if typed pwd
} //controls builtin such as exit, pwd, cd


int main(int argc, char **argv)
{
	char c;
	char arr[512],complete[512];
	int i = 0, flag = 0;
	int input_char = 0;
	/* Switch to non-canonical terminal mode */
	termios_set_noncanonical();

	memset(&arr, 0, sizeof(arr)); //clears array before going into shell loop
	memset(&complete, 0, sizeof(complete));

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
			if (input_char == 0) { //there's nothing for input
				write(STDOUT_FILENO, "\nsshell$ ", 9);
			}
			else {
				strncpy(complete, arr, sizeof(complete));
				putchar('\n');
				fflush(stdout);
				

				if (history_count<10) { //there are up to 9 history
					memcpy(history[history_count], &arr, sizeof(arr)); //something wrong here
					history_count++;
					history_current++;
					
					//this makes sure that we are always on the last command
					if (history_count == 10) {
						history_current = 9;
					}
					else {
						history_current = history_count;
					}
				}
				else { //there are 10 history already
					for (int i=0;i<9;i++) {
						//the newer history replace older ones
						for(int j=0;j<sizeof(history[i]);j++) {
							history[i][j] = 0;
						}
						memcpy(history[i],history[i+1],sizeof(history[i+1]));
					}
					//add the user input to the newest history
					memcpy(history[9],arr,input_char+1);
				}
				execute_command(arr,input_char);
				write(2, "+ completed '", 13);
				write(2, &complete, sizeof(complete));
				write(2, "'", 1);
				write(2, " [", 2);
				write(2, &completeflag, 1);
				write(2, "]\n", 2);
				completeflag = '0';
				if(exit_flag == 1)
					return 0;
				input_char = 0;
				memset(&arr, 0, sizeof(arr));
				write(STDOUT_FILENO, "sshell$ ", 8);
			}
		}
			

		else if (c == 0x7F) { //backspace
			write(STDOUT_FILENO, &c, 1);
			memset(&arr[input_char-1 ], 0, 1);
			input_char--;
		}

		else if (c == 0x1B) { //first part of arrow
			c = get_one_char();
			if (c == 0x5B) { //second part of arrow
				c = get_one_char();
				char* delete_char = (char*)0x7F;

				if (c == 0x41) { //up-arrow
					//write(2,"up-arrow\n",9);
					//check if there are history to go back
					//printf("history_count is: %d\n",history_count);
					//printf("history_current is: %d\n",history_current);

					if (history_count>0 && history_current>0) {
						//write(2,"up-arrow\n",9);
						//delete current input
						//printf("sizeof input_char is: %d\n",input_char);
						for (int i=0;i<input_char;i++) { 
							write(STDOUT_FILENO, &delete_char, 1);
							memset(&arr[i], 0, 1);

						}
						//memcpy(arr,0,sizeof(arr));

						//load the previous history
						history_current--;
						write(2,history[history_current],sizeof(history[history_current]));
						memcpy(&arr,history[history_current],sizeof(history[history_current]));
						input_char = strlen(history[history_current]);
					}
					//add sound
					else if (history_current == 0) {
						write(2,"\a",1);
					}
				}

				else if (c == 0x42) { //down-arrow
					//write(2,"down-arrow\n",11);
					//check if there are more recent history
					if (history_count<11) {

						if (history_current<history_count) { //make sure we don't go past last history
							for (int i=0;i<input_char;i++) { 
								write(STDOUT_FILENO, &delete_char, 1);
								memset(&arr[i], 0, 1);
							}
							//memcpy(arr,0,sizeof(arr));

							//load the next history
							history_current++;
							write(2,history[history_current],sizeof(history[history_current]));
							memcpy(&arr,history[history_current],sizeof(history[history_current]));
							input_char = strlen(history[history_current]);
						}
						//add sound
						else if (history_current == history_count-1) {
							write(2,"\a",1);
						}
					}
				}
			}
		}
	}


	/* Switch back to previous terminal mode */
	termios_reset();

	return EXIT_SUCCESS;
}


