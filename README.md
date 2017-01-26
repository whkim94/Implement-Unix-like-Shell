# Pro1
We developed the code from noncanmode.c and worked from there.

In our main() on line 379, we seperate into different cases depending on what the user types:

  1. The user presses ctrl-D, then we exit
  2. the user enters a character that is printable, then we add it to the current prompt and waits for next input
  3. the user presses enter, then we go into execute_command() to parse the input and execute them, and then we return the result
  4. the user presses backspace, then we delete one character from the prompt and waits for next input
  
  
In our execute_command() on line 267, we parse the command and check for the following cases
  1. there's a "|", so we go to pipline()
  2. there's a ">" or "<", so we go to redirection() 
  3. otherwise it's just one command that we need to execute, then we check:
    1. the command is one of "exit" or "pwd" or "cd", then we go into builtin_command()
    2. all other commands are forked and executed with execvp()

In our builtin_command() on line 339, we execute the command depending on what it is:
  1. "exit" -- we just return and set the exit_flag to 1 so that main() knows we are exiting
  2. "cd"   -- if there's nothing after "cd", then we go to home directory of the user   
               else we go to the directory specified by the user
  3. "pwd"  -- we write out the current directory
  
In our pipeline() on line 97, we do something:

In our redirection() on line 177, we do something:
