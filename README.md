# Pro1
### Basic info:
We developed the code from noncanmode.c and worked from there.
As a group, we meet up often and every once in a while would combine all our work into one file, then everyone works off that file until next time we merge again.


### Functions:
In our main(), we seperate into different cases depending on what the user types:
  1. The user presses ctrl-D, then we exit
  2. the user enters a character that is printable, then we add it to the current prompt and wait for next input
  3. the user presses enter, then we go into execute_command() to parse the input and execute them, and then we return the result
  4. the user presses backspace, then we delete one character from the current prompt and wait for next input
  
  
In our execute_command(), we parse the command and check for the following cases:
  1. there's a "|", so we go to pipline()
  2. there's a ">" or "<", so we go to redirection() 
  3. otherwise it's just one command that we need to execute, then we check:
    1. the command is one of "exit" or "pwd" or "cd", then we go into builtin_command()
    2. all other commands are forked and executed with execvp()

In our builtin_command(), we execute the command depending on what it is:
  1. "exit" -- we just return and set the exit_flag to 1 so that main() knows we are exiting
  2. "cd"   -- 
    * if there's nothing after "cd", then we go to home directory of the user   
    * else we go to the directory specified by the user
  3. "pwd"  -- we write out the current directory
  
In our pipeline(), we try a few ways to implement the function:
  1. One of our function in the code take the number of argument in pipeline, for ex., ls|cat = 2 words. For the number of words, we loop through andpipe and fork in every loop. We first make sure that we have the right command to pass to the exec function. Once we pipe, we try to send the old output to the new input. The proble we encountered, was either it would hang on the last run or for commands such as ls|cat, "we would get cd : '' : no such file or directory".
  2. If there is pipe in the command, it enters the pipe function we implemented. Then parse the array based on strtok of "|" and get the first process into while loop. It is supposed to get the outcome of very first process going through execute_command() function that we implemented in order to deal with all different kinds of commands. Then the dup2 should get whatever the output and pass to parent. Parent process should notice if there is another pipe after the array. If there is another pipe, then it should store the result of execution into fd then go to beginning of while. If there is no pipe, then call execvp to finalize our command with the final result.

In our redirection() on line 177, we do check whether it's a ">" or "<":
  1. If it's a ">": we read in the argument before ">" and execute it, then pass it to the file. We also have to create the file and write in it by using modes (O_CREAT | O_RDWR | S_IROTH)
  2. If it's a "<": we do the same except we open the file on the right side of "<" by using mode (O_RDONLY), then we pass it in as arguments for the command on the left side of "<" and execvp()

### To-Do List
- [x] Get a line with fget() (We did it differently)
- [x] Parse command lines
- [x] Execute commands and wait for it to finish
- [x] Handle built-in functions
- [x] Accept optional arguments
- [ ] Piping
- [x] Redirection
- [ ] Background commands
- [x] History management

### Resources we found:
  * http://stackoverflow.com/questions/17630247/coding-multiple-pipe-in-c (used in pipline() )
  * http://stackoverflow.com/questions/15798450/open-with-o-creat-was-it-opened-or-created ( used in redirection() )
  * http://www.cplusplus.com/reference/cstring/strchr/ (used in execute_command() to check if there are "|" or ">" or "<")
  * http://pubs.opengroup.org/onlinepubs/009695399/functions/getlogin.html (used for cd and pwd to get username that's logged in)
  * http://stackoverflow.com/questions/4785126/getlogin-c-function-returns-null-and-error-no-such-file-or-directory (same as above)
  * https://linux.die.net/man/3/getcwd (used get_current_dir_name(), which doesn't require parameter compared to getcwd() )
  
### Notes:

  
