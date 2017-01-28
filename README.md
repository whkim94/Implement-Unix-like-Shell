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
  
In our pipeline(), we do something:

In our redirection() on line 177, we do check whether it's a ">" or "<":
  1. If it's a ">": we read in the argument before ">" and execute it, then pass it to the file. We also have to create the file and write in ti by using modes (O_CREAT | O_RDWR | S_IROTH)
  2. If it's a "<": we do the same except we open the file on the right side of "<" by using mode (O_RDONLY), then we pass it in as arguments for the command on the left side of "<" and execvp()


### Resources we found:
  * http://stackoverflow.com/questions/17630247/coding-multiple-pipe-in-c (used in pipline() )
  * http://stackoverflow.com/questions/15798450/open-with-o-creat-was-it-opened-or-created ( used in redirection() )
  * http://www.cplusplus.com/reference/cstring/strchr/ (used in execute_command() to check if there are "|" or ">" or "<")
  * http://pubs.opengroup.org/onlinepubs/009695399/functions/getlogin.html (used for cd and pwd to get username that's logged in)
  * http://stackoverflow.com/questions/4785126/getlogin-c-function-returns-null-and-error-no-such-file-or-directory (same as above)
  * https://linux.die.net/man/3/getcwd (used get_current_dir_name(), which doesn't require parameter compared to getcwd() )
  
### Notes:

  
