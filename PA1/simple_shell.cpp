#include <stdlib.h>	/* needed to define exit() */
#include <unistd.h>	/* needed to define getpid() */
#include <stdio.h>	/* needed for printf() */
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>
#include <signal.h>
using namespace std;

#define STD_INPUT 0
#define STD_OUTPUT 1

void func(int signum) 
{ 
    wait(NULL); 
} 

void printDir(){
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	printf("\n%s", cwd);
}

bool shell(int argc, char *argv[]){
	if(argc == 2){
		if(string(argv[1])=="-n"){
			return true;
		}
	}
	printf("shell: ");
	return true;
}

int changeDirectory(char* args[]){
	// If we write no path (only 'cd'), then go to the home directory
	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}
	// Else we change the directory to the one specified by the 
	// argument, if possible
	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s does not\n", args[1]);
            return -1;
		}
	}
	return 0;
}


void exec(string command){
	char ch = command.back();
    int n = ch-'0';
	command.pop_back();
    char *char_array[n+1];
	stringstream ss(command);
	string temp;
	int c=0;
	// char_array[0] = (char *)"cat";
	while(ss >> temp){
		// cout << temp << ", ";
		char_array[c] = strdup(temp.c_str());
		c++;
	}
	char_array[c] = (char *)0;
	// cout << "\n";
	// execvp(char_array[0],char_array);
	// char *char_arrayy[] = {(char *)"ls"};
	if (strcmp(char_array[0],"cd") == 0) changeDirectory(char_array);
	else{
		execvp(char_array[0],char_array);
		perror("ERROR");
		return;
	}
}

void spawn_proc (int in, int out, string cmd){
  	pid_t pid;
  	if ((pid = fork ()) == 0){
      	if (in != 0){
        	dup2 (in, 0);
        	close (in);
        }
      	if (out != 1){
        	dup2 (out, 1);
        	close (out);
        }
      	exec(cmd);
    }
  	// return pid;
}

void fork_pipes (int n, string cmds[], bool out, bool input, char * outputFile = NULL, char * inputFile = NULL){
	int fileDescriptor;
	//char * inputFile = "1"
	if (input == 1){
		fileDescriptor = open(inputFile, O_RDONLY, 0600);  
		if(fileDescriptor<0){
			perror("ERROR");
			return;
		}
		dup2(fileDescriptor, STDIN_FILENO);
		close(fileDescriptor);
	}
	int i;
	pid_t pid;
	int in, fd [2];
	in = 0;
	if(n==2 && input && !out){
		exec(cmds[0]);
		return;
	}
	if(n==2 && input && out){
		i=0;
	}
	else{
		for (i = 0; i < n - 1; ++i){
			if(i==1 && input){
				continue;
			}
			pipe (fd);
			spawn_proc (in, fd [1], cmds[i]);
			close (fd [1]);
			in = fd [0];
		}
		if (in != 0){
			dup2 (in, 0);
		}
	}
	//char* outputFile = "output.txt";
	if (out == 1){
			// We open (create) the file truncating it at 0, for write only
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
			// We replace de standard output with the appropriate file
			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);
	}
	exec(cmds[i]);
	return;
}


void parseParts(int partc, char* partv[], string* cmd, int& cmdc, char *relation, int& relationc){
	// string temp[8];
	string temp;
	int tempc=0;
	char cread = 'r';
	char cwrite = 'w';
	char cpipe = 'p';
	for(int i = 0; i<partc;i++){
		if(partv[i]!=(char *)"<" && partv[i]!=(char *)">" && partv[i]!=(char *)"|"){
			// temp[tempc] = (string)partv[i];
			tempc++;
			temp = temp + " " + (string)partv[i];
		}
		else{
			// for(int j=0;j<tempc;j++){
			// 	cmd[cmdc]= temp[j];
			// 	temp[j] = "";
			// }
			temp.append(to_string(tempc));
			cmd[cmdc] = temp;
			temp = "";
			cmdc++;
			tempc=0;
			if(partv[i]==(char *)"<"){ //read
				relation[relationc] = cread;
				relationc++;
			}
			else if(partv[i]==(char *)">"){ //write
				relation[relationc] = cwrite;
				relationc++;
			}
			else{ //pipe
				relation[relationc] = cpipe;
				relationc++;
			}
		}
	}
	temp.append(to_string(tempc));
	cmd[cmdc] = temp;
	cmdc++;
}


//runtime_error count_lt("ERROR: Too many <");
#define BUFFERSIZE 512

void handle_sigint(int sig) 
{ 
   return;
} 
  

int main(int argc, char *argv[]) {
	signal(SIGINT, handle_sigint); 
	
	//char *args[] = {(char *)"cat",(char *)"<", (char *)"1", (char *)0};	/* each element represents a command line argument */
 	// char *args[] = {"lst", 0};	/* each element represents a command line argument */
	// char *args[] = {"echo", "$PATH", 0};	/* each element represents a command line argument */
	char *env[] = { 0 };	/* leave the environment list null */
	

	char buffer[BUFFERSIZE];
	string buffer_spaced;
    char cChar;
	string delimiters(" \n");
	string metachars = "<>|&";
    while(shell(argc, argv) && fgets(buffer, BUFFERSIZE, stdin) != NULL){
		string fixed;
		string temp = "";
		int count_lt=0, count_gt=0, count_amp=0, count_pipe=0;
		vector<string> parts;
		char *partv[64];
		int partc = 0;
		char *cstr;
		for (int i= 0; buffer[i] != '\0'; ++i){
			
			switch(buffer[i]){
				case '<':{
					if(temp!=""){
						cstr = new char[temp.size()+1];
						strcpy(cstr, temp.c_str());
						partv[partc] = cstr;
						partc++;
					}

					partv[partc] = (char *)"<";
					partc++;
					count_lt++;
					temp="";
					break;
				}
				case '>':{
					if(temp!=""){
						cstr = new char[temp.size()+1];
						strcpy(cstr, temp.c_str());
						partv[partc] = cstr;
						partc++;
					}
					partv[partc] = (char *)">";
					partc++;
					count_gt++;
					temp="";
					break;
				}
				case '|':{
					if(temp!=""){
						cstr = new char[temp.size()+1];
						strcpy(cstr, temp.c_str());
						partv[partc] = cstr;
						partc++;
					}
					partv[partc] = (char *)"|";
					partc++;
					temp="";
					count_pipe++;
					break;
				}
				case '&':{
				 	if(temp!=""){
				 		cstr = new char[temp.size()+1];
				 		strcpy(cstr, temp.c_str());
				 		partv[partc] = cstr;
						partc++;
				 	}
				 	partv[partc] = (char *)"&";
				 	partc++;
				 	count_amp++;
					break;
				}
				case ' ':{
					
					if(temp!=""){
						cstr = new char[temp.size()+1];
						strcpy(cstr, temp.c_str());
						partv[partc] = cstr;
						partc++;
					}
					temp="";
					break;
				}
				default:{
					
					if(buffer[i+1]=='\0' && temp!=""){
						
						cstr = new char[temp.size()+1];
						strcpy(cstr, temp.c_str());
						partv[partc] = cstr;
						partc++;
					}
					else{
						temp+=buffer[i];
					}
					break;
				}
			}
		}
		if(partc==0){
			continue;
		}
		if(count_lt>1){
			cout << "ERROR: More than 1 \"<\"\n";
			continue;
		}
		if(count_gt>1){
			cout << "ERROR: More than 1 \">\"\n";
			continue;
		}
		if(count_amp>1){
			cout << "ERROR: More than 1 \"&\"\n";
			continue;
		}
		if(count_amp==1 && partv[partc-1]!="&"){
			cout << "ERROR: \"&\" not in correct place\n";
			continue;
		}
		char * outputfile;
		if(count_amp==1){ //& is present
			outputfile = partv[partc-2];
			partc--;
		}
		else{
			outputfile = partv[partc-1]; 
		}
		string cmd[256];
		int cmdc=0,relationc=0;
		char relation[256];
		// cout << "\n";
		bool out = 0;
		bool input = 0;

		parseParts(partc, partv, cmd, cmdc, relation, relationc);
		int checkifwrite = 0;
		if (relation[relationc-1] == 'w'){
			checkifwrite = 1;
			out = 1;
		}
		char * inputfile;
		if (relation[0] == 'r'){
			input = 1;
			//cout <<partv[2];
			inputfile = partv[2];
		}


		
		int flag=0;
		for(int i=0;i<relationc;i++){
			if(relation[i]=='r' && i!= 0){
				cout << "ERROR: Only the first command can have it's input redirected\n";
				flag++;
				break;
			}
			else if(relation[i]=='w' && i!= relationc-1){
				cout << "ERROR: Only the last command can have it's output redirected\n";
				flag++;
				break;;
			}
		}
		if(flag>0){
			continue;
		}
		int status;
		if(fork() > 0){
			// cout << "parent\n";
			if(count_amp==1){
				signal(SIGCHLD, func); 
			}
			else{
				waitpid(-1,&status,0);
			}
		}
		else{
			// cout << "child\n";
			fork_pipes(cmdc-checkifwrite, cmd, out, input, outputfile, inputfile);
			continue;
		}



	}
	// printf("About to run cat 1\n");
	// execvp("cat", args);
	// execvp("lst", args);	
	// execvp("echo", args);
	// perror("ERROR: ");	/* if we get here, execvp failed */
	// execve("cat", args, env);
	// perror("execve");	/* if we get here, execvp failed */
	return 0;
}