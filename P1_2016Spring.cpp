#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

void parseString(string input);
void clearLine(string input);
void myFork( vector<string> input);
void myls(string input);
void myff(string query, string place, string dirAns, int depth);

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}



int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    string currentInput = "";//saving input in here
    list<string>  history;
    int histPlaceEnd = 0;
    //list<string>::iterator histItEnd = history.begin();//for the end of the list
    list<string>::iterator histIt = history.begin();//for where the user wants to be
    int histPlace = 0; // to help keep track of the iterators location so it doesnt go out of bounds

    history.push_front(currentInput);//making blank spot

    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
	
    char * cwdbuffer; 
    char* cwd = "";

    cwd = getcwd(cwdbuffer,100); //may need to be bigger
    write(1, cwd, strlen(cwd));  	    
    write(1,"%",1);
    read(STDIN_FILENO, &RXChar, 1);
    while(1){
	//cout << "START OF WHILE" << endl;
        if(0x04 == RXChar){ // C-d
            break;
        }
        else{
            if(isprint(RXChar)){//inputing line 
				currentInput +=RXChar;
                		write(1, &(RXChar) , 1);
				read(STDIN_FILENO, &RXChar, 1);
            }
            else if (RXChar == 0x0A){//hit enter 
				//the case of return, so need to run the inputed line 
				
				parseString(currentInput);//parse the input
						
			//		histIt = history.begin();//resetting to begining, so next "round" of input will be able to iterate through history right
			//	histPlace = 0;

				if(histPlaceEnd < 10){//if there is not 10 commands yet
					histPlaceEnd += 1;
					history.pop_front();//getting rid of the blank
					history.push_front(currentInput);//pushing current input
				}
				else{
					history.pop_front();		
					history.push_front(currentInput);
					history.pop_back();
				}
				currentInput = "";
				history.push_front(currentInput);
				histIt = history.begin();//resetting for next round	
				histPlace = 0;
				write(1, "\r\n", 1);
				cwd = getcwd(cwdbuffer,100);
				write(1	, cwd, strlen(cwd));
				write(1,"%",1);			
				read(STDIN_FILENO, &RXChar, 1);
			}
	     else if(RXChar == 0x1B){//is an arrow
				read(STDIN_FILENO, &RXChar, 1);//reading the next char, will check if it was an arrow
				if(RXChar == '['){//else if arrow key
					
					 read(STDIN_FILENO, &RXChar, 1);//finding the arrow decider, need switch statement

					 if(RXChar == 'A'){
						if (histPlace < 11 && histPlace < histPlaceEnd){

							clearLine(currentInput);

							histPlace += 1;
							++histIt;
							currentInput = *histIt;//need to clear buffer 
							write(1, currentInput.c_str(), currentInput.size());
							
							
						}
						//else play sound
					}
					else if (RXChar == 'B'){//downarrow if at not the last spot on list, move down and iterate down the list
						if(histPlace > 0){
							clearLine(currentInput);

							histPlace -= 1;
							--histIt;
							currentInput = *histIt;//need to clear buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
							write(1, currentInput.c_str(), currentInput.size());
						}
						else {
						//	cout << "dA at edge" << endl;//make sound
						}
					}
					else if(RXChar == 'C' || RXChar == 'D'){//do nada
					}
					else{
						cout << "SHIT!" << endl;
					}
					
					read(STDIN_FILENO, &RXChar, 1);//reading passed the arrow decider
					
					
				}
				
			}
	     else if(RXChar == 0x7F){//delete
				 	if(currentInput.size() > 0){
						write(1, "\b \b", strlen("\b \b"));
						currentInput.erase(currentInput.end() - 1);
						//cout << "Current INPUT" << currentInput << endl;
					}
					read(STDIN_FILENO,&RXChar, 1);
			
			}
		//NEED TO CHECK FOR "TAB, which is Ox09, and not do anything. enter is 0x0A
	   else {
		cout << "check on this output, this is not supposed to happen" << endl;
	  }
        }
	
          
    }
    
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;}

void parseString(string input){
   vector<string> temptokens;
   vector<vector<string> > tokens;//to store pipe commands. could find better way prolly
   bool word = false;
   int pipeCounter = 0;
   string newWord = "";
   /*for(int i = 0; i < input.size(); i++){// own parser can reuse fo piping and stuff
		if(input[i] != ' ' && input[i] != '>' && input[i] != '<'){
			if(in == true){//for redirect
				newWord += '>';	
				in = false;
			}
			if(out == true){//for redirect
				newWord += '<';
				out = false;
			}
			word = true;
			newWord +=input[i];
		}
		else if(input[i] == ' ' && word == true){
			temptokens.push_back(newWord);
			newWord = "";
			word = false;
		}
		else if (input[i] == ' ' && word == false ){
			
		}
		else if (input[i] == '>'){
			in = true;
		}
		else if (input [i] == '<'){
			out = true;	
		}*/
		
//].c_str(), O_CREAT | O_WRONLY);
//dup2(fd, STDOUT_FILENO);
			


//int fd = open(input[i+1].c_str(), O_CREAT | O_WRONLY);
		
	}
   if(word == true)
	temptokens.push_back(newWord);

  newWord == "";
  tokens.push_back(temptokens);	//testing	 
  for(unsigned int i = 0; i < tokens[0].size(); i++){
	cout << i << " spot in Vector:" << tokens[0][i] << endl; 
  }
  
if (tokens[0][0]=="cd") { //case of cd

    if (tokens[0].size()==1) {
      const char *start = "/home/";
      char* userName = getlogin();
      //cout<<userName<<endl;
      int nameSize = sizeof(userName)/sizeof(userName[0]);
      char *homeDestination = new char[6+nameSize];
      strcpy(homeDestination, start);
      strcat(homeDestination, userName);
    
      //cout<<"changing to home directory:"<<destination<<endl;
      //destination = "/home/jlee1819";
      int cd_result = chdir(homeDestination);
      if (cd_result == -1) {cout<<"File/Directory not found"<<endl;}
      
    }
    else {
       cout<<"token size:"<<tokens[0][1].size()<<endl;
       char *destination = new char[tokens[0][1].size()];
       //strcpy(destination,tokens[0][1]);
       for (int i=0;i<tokens[0][1].size();i++) {
	 destination[i] = tokens[0][1][i];
	 cout<<destination<<endl;
       }
       
       cout<<"tokens is:"<<tokens[0][1]<<endl;
       cout<<"change to:"<<destination<<endl;
       int cd_result = chdir(destination);
       if (cd_result == -1) {cout<<"Directory not found"<<endl;}
    }
  }
 else if (tokens[0][0]=="exit") {
   exit(0);
  }
 else {  
   for(unsigned int i = 0; i < tokens.size(); i++){//not cd, so need to fork it
	myFork(tokens[i]);
   }
 }

} 
  
void myFork(vector<string> input){

//      cout << "IN FORK YOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" << endl;
      pid_t pid = fork();
      int status = 0;
      if(pid == 0){  
			if (input[0]=="ls") {    //case of ls
				if (input.size() >1){
					cout << "in " <<endl;
					myls(input[1]);
				}
				else
					myls("");           
				exit(0);
			 }
		  
			 else if (input[0]=="pwd") {   //case if pwd
				cout << "in pwd" << endl;	
				char* dir_name = get_current_dir_name();
				write(1, dir_name, strlen(dir_name));
				write(1,"\r\n", 2);
				exit(0); 	
			  }	  
				
			  else if (input[0] == "ff"){
				if (input.size() == 2){
								cout << "in " <<endl;
								myff(input[1],"", "",0);
				  }
				else if(input.size() > 2)
						myff(input[1],input[2],"",0);
				else
						write(1,"ff command requires a file name!\n", 33);
				exit(0);
			}
			else if (input[0] == "exit"){//need to make better 
				exit(0);
			}
			else if (input[0] == "cd") {} //do nothing

			else { //commands we can't handle internall

			  //http://stackoverflow.com/questions/20390008/how-to-convert-stdstring-to-const-char-in-c
			  //char *const args = input.c_str();
			  
			  //https://www.youtube.com/watch?v=O1UOWScmqxg
			  
			  if (pid == 0) { //this is a child
			        //cout<<"pid of child:"<<pid<<endl;
				char* args[20];
				//string mycmd = tokens[0];
				//string myarg = tokens[1];
				//args[0] = (char*)mycmd.c_str();
				//args[1] = (char*)myarg.c_str();
				
				for (int i = 0; i<20;i++) {
					args[i] = NULL;
				}
				for (int i=0;i<input.size();i++) {
					args[i] = (char*)input[i].c_str();
				}
				if (execvp(args[0],args) == -1) {
					perror("exec");
				}
			  }
			}
	}
     else if (pid > 0){
		while(wait( &status )>0){
		}
     }
     else{
		cout << "ONON"<<endl;
	 }	

   
}

void myls(string input){
	if(input == ""){
		cout << "HAHAH" << endl;
		input = get_current_dir_name();
	}

	DIR* drcty;
	struct dirent* entry;
	struct stat fileStat;
	
	if(drcty = opendir(input.c_str())){
	string permissions = "";
        string addInput = "";
	while( (entry = readdir(drcty)) != NULL){
	    
	    addInput += (input + "/" + entry->d_name);  
            //cout << "addinput " << addInput << endl;  
	    if(stat(addInput.c_str(), &fileStat)==0){// for stat to work the files in the path var (addInput, in this case) must have permission to be searched. BE CAREFUL
	    permissions += ((S_ISDIR(fileStat.st_mode))  ? "d" : "-");
            permissions += ((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            permissions += ((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            permissions += ((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            permissions += ((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            permissions += ((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            permissions += ((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            permissions += ((fileStat.st_mode & S_IROTH) ? "r" : "-");
            permissions += ((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            permissions += ((fileStat.st_mode & S_IXOTH) ? "x" : "-");	
	    write(1, permissions.c_str(), strlen(permissions.c_str()));
	    write(1, entry->d_name, strlen(entry->d_name));
	    write(1, "\r\n", 2);
	    permissions = "";
	    addInput = "";
	    	
	    }				
	}
	}
	else{
		write(1,"ls: cannot access ", strlen("cannot access "));
		write(1,input.c_str(), strlen(input.c_str()));
	}  		
	closedir(drcty);	
	
}

void myff(string query, string place, string dirAns, int depth){//query is what we are lookiing for, place is the directory we are searching in. dirAns is for the answer output an depth iis how far recursively weve gone
	if (place == "" ){
		place = get_current_dir_name();
		dirAns = "./";
	}
	else if (place[0] == '/'&& depth == 0){
		dirAns = place + '/';	
	}
	else if (depth == 0){
		dirAns = "./";
	}
	
	DIR* drcty;
	struct dirent* curEntry;
	struct stat fileStat;
	string addInput = "" ;//used to cycle through the entries and call the ff recursively
	string output = ""; //used for answer
	
	if(drcty = opendir(place.c_str())){
	
		while( (curEntry = readdir(drcty)) != NULL){
			//cout << "IN WHILE LLOOOP " <<curEntry->d_name <<  endl;
			addInput = (place + "/" + curEntry->d_name);
			if(stat(addInput.c_str(), &fileStat)==0){
				if (fileStat.st_mode & S_IFDIR){
					if(!(strcmp(curEntry->d_name, "." )==0) && !(strcmp(curEntry->d_name, "..")==0)){
			//			cout << "DIR "<< curEntry->d_name << endl;
						myff(query, addInput, dirAns+ curEntry->d_name+"/", depth+1);
					}
				}
				else if ( fileStat.st_mode & S_IFREG){
					if (query == curEntry->d_name){
						//cout <<"YEE                                                                           " ;	
						output = dirAns +(curEntry->d_name);
						write (1,output.c_str(), strlen(output.c_str()));
						write(1, "\r\n",2);
						output = "";
					}
				}
			}
			addInput = "";
		}		
	}
	else {
		write(1, place.c_str() , strlen(place.c_str()));
		write(1, " C\n ", 3);
	}
	closedir(drcty);	
}

void myRedirect(vector<string>* input){

	int i = 0;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	while( i < (*input).size()){
		if((*input)[i][0] == '<'){
			
			(*input)[i] =  (*input)[i].substr(1,(*input)[i].size()).c_str();
			int fd = open( (*input)[i].c_str(), O_RDONLY, mode);
			
                       (*input).erase((*input).begin() + i);
			dup2(fd,0);
			close(fd);
			i--;//getting back to right spot for iterator	
		}
		else if((*input)[i][0] == '>'){
			cout << "JOUT" << endl;
			
			(*input)[i] = (*input)[i].substr(1,(*input)[i].size()).c_str();
			int fd = open( (*input)[i].c_str(), O_WRONLY | O_CREAT|O_TRUNC, mode);
                        (*input).erase((*input).begin() + i);
			dup2(fd,1);
                        dup2(fd,2);
                        close(fd);
			i--;
		}
		i++;
	}

}



void clearLine(string input){

	for(int i = 0; i < input.size(); i++){
		write(1, "\b \b", strlen("\b \b"));
	} 

}

