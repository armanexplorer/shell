// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<dirent.h>
#include<fnmatch.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h> 
#include<sys/wait.h> 
#include<errno.h>
#include<readline/readline.h> 
#include<readline/history.h> 

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

/**
'\033' stands for ESC that is the octal form (in C programming language) of 27 ANSI value of ESC 

'ESC[' is kind of escape sequence called CSI (Controll Sequence Intorducer)
'ESC[H' and 'ESC[j' are CSI codes:
'ESC[H' moves the cursor to the top left corner of screen
'ESC[j' clears the part of screen between cursor to the end of screen
*/

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J") 

pid_t MAIN_PID;

// Signal Handler
void terminate_signal_handler(int signum){
	if(getpid() != MAIN_PID){
		exit(0);
	}
}


// Greeting shell during startup 
void init_shell() 
{ 
	clear();
} 


// Function to get prompt text
void getPromptIntro(char *intro_text) 
{ 
	char str[300] = "\nplease enter a command>";
	//char str[300] = "\n(;ASH;) ";
	//char cwd[200]; 
	//getcwd(cwd, sizeof(cwd)); 
	//strcat(str, "\n(;ASH;) ");
	//strcat(str, cwd);
	//strcat(str, "# ");
	strcpy(intro_text, str);
} 


// Function to print Current Directory.
/**
void printDir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf(CYN "\n(;ASH;) " RESET);
	printf("%s:", cwd);
}
*/

// Function to take input 
int takeInput(char* str) 
{ 
	char* buf;
	char intro_text[300]; 
	getPromptIntro(intro_text);
	buf = readline(intro_text); 
	if (strlen(buf) != 0) { 
		add_history(buf); 
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

// Function where the system command is executed 
void execArgs(char** parsed) 
{ 
	// Forking a child 
	pid_t pid = fork(); 

	if (pid == -1) { 
		printf(RED "Failed forking child..\n", 30); 
		return; 
	} else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf(RED "Could not execute command..\n", 30); 
		} 
		exit(0); 
	} else { 
		// waiting for child to terminate 
		wait(NULL); 
		return; 
	} 
} 

// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
	// 0 is read end, 1 is write end 
	int pipefd[2]; 
	pid_t p1, p2; 

	if (pipe(pipefd) < 0) { 
		printf(RED "Pipe could not be initialized\n", 30); 
		return; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf(RED "Could not fork\n", 30); 
		return; 
	} 

	if (p1 == 0) { 
		// Child 1 executing.. 
		// It only needs to write at the write end 
		close(pipefd[0]); 
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[1]); 

		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command 1.."); 
			exit(0); 
		} 
	} else { 
		// Parent executing 
		p2 = fork(); 

		if (p2 < 0) { 
			printf(RED "Could not fork\n", 30); 
			return; 
		} 

		// Child 2 executing.. 
		// It only needs to read at the read end 
		if (p2 == 0) { 
			close(pipefd[1]); 
			dup2(pipefd[0], STDIN_FILENO); 
			close(pipefd[0]); 
			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				printf(RED "Could not execute command 2..\n" ,30); 
				exit(0); 
			} 
		} else { 
			// parent executing, waiting for two children 
			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 

// Help command builtin 
void openHelp() 
{ 
	puts("\n***WELCOME TO MY SHELL HELP***"
		"\nCopyright @ Suprotik Dey"
		"\n-Use the shell at your own risk..."
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>improper space handling"); 

	return; 
} 

// Function to execute builtin commands 
int ownCmdHandler(char** parsed) 
{ 
	int NoOfOwnCmds = 8, i, switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	char ash_text[30] = "this is the sound of ash :\0"; 

	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "help"; 
	ListOfOwnCmds[3] = "ash"; 
	ListOfOwnCmds[4] = "ls";
	ListOfOwnCmds[5] = "lsdir";
	ListOfOwnCmds[6] = "rename";
	ListOfOwnCmds[7] = "badloop"; // To show singal handling works perfectly

	for (i = 0; i < NoOfOwnCmds; i++) { 
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 

	pid_t p1;
	switch (switchOwnArg) { 
	case 1:  // exit
		printf("\nGoodbye\n"); 
		exit(0); 
	case 2:  // cd
		if(parsed[2] != NULL){
			printf(RED "illegal number of arguments\n" , 30);
			return 1;
		}
		if(chdir(parsed[1]) == -1){
			if(errno == ENOENT)
				printf(RED "directory '%s' does not exist\n", parsed[1], 30);
		} 
		return 1; 
	case 3:  // help
		openHelp(); 
		return 1; 
	case 4:  // ash
		for(int j=0;j<sizeof(ash_text);j++){
			printf("%c",ash_text[j]);
			fflush(stdout);
			usleep(100000);
		}
		printf(CYN ")\n" RESET);
		fflush(stdout);
		sleep(1);
		return 1; 
	case 5: ;// ls
		p1 = fork();
		if(p1<0){
			printf(RED "An error occured in forking", 30);
			return 1;
		}
		else if(p1==0){
			i = 0;
			int has_hide = 0;

			while(parsed[i] != NULL){
				if ( strcmp(parsed[i], "--hide" )==0){
					parsed[i] = NULL;
					has_hide = 1;
					break;
				}
				i++;
			}
			if(has_hide == 0){
				parsed[i] = "-a";
				parsed[i+1] = NULL;
			}
			if (execvp(parsed[0], parsed) < 0) { 
				printf(RED "Could not execute command..\n", 30);
			}
			exit(0); 
		}
		else{
			wait(NULL);
			return 1;
		}
	case 6: ;//  lsdir
		p1 = fork();
		if(p1<0)
			perror(RED "forking failed");
		else if(p1==0){
			struct dirent **namelist;
			struct stat buf;
		       	int n;

			i = 0;
		       	n = scandir(".", &namelist, NULL, alphasort);
	       		if (n < 0)
	       		    	perror(RED "scan directory failed"); // 'perror' PUTS LAST ERROR MESSAGE IN FRONT OF THE GIVEN TEXT!!!!
				//printf(RED "scan directory failed");
	       		else {
				if(parsed[1] == NULL){
		           		while (i<n) {
						stat(namelist[i]->d_name, &buf);
						if(S_ISDIR(buf.st_mode))
		               				printf("%s\n", namelist[i]->d_name);
		           			free(namelist[i]);
						i++;
		           		}
		           		free(namelist);
				}
				else{
					while (i<n) {
						stat(namelist[i]->d_name, &buf);
						if(S_ISDIR(buf.st_mode) && fnmatch(parsed[1], namelist[i]->d_name, 0) == 0)
		               				printf("%s\n", namelist[i]->d_name);
		           			free(namelist[i]);
						i++;
		           		}
		           		free(namelist);
				}
      		 	}
			exit(0);
		}
		else{
			wait(NULL);
			return 1;
		}
	case 7: //rename
		if(parsed[1] != NULL && parsed[2] != NULL && parsed[3] == NULL)
			rename(parsed[1], parsed[2]);
		else
			printf(RED "illegal number of arguments\n",30);
		return 1;
	case 8: ;//badloop
		p1 = fork();
		if(p1<0){
			perror(RED "the forking failed");
		}
		else if(p1==0){
			while(1){
				printf(YEL "we are in badloop\n" RESET);
				sleep(2);
			}
		}
		else{
			wait(NULL);
			return 1;
		}
	default: 
		break; 
	} 

	return 0; 
} 

// function for finding pipe 
int parsePipe(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; // returns zero if no pipe is found. 
	else { 
		return 1; 
	} 
} 

// function for parsing command words 
void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

int processString(char* str, char** parsed, char** parsedpipe) 
{ 

	char* strpiped[2]; 
	int piped = 0; 

	piped = parsePipe(str, strpiped); 

	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe); 

	} else { 

		parseSpace(str, parsed); 
	} 

	if (ownCmdHandler(parsed)) 
		return 0; 
	else
		return 1 + piped; 
} 

int main() 
{ 
	MAIN_PID = getpid();
	signal(SIGINT, terminate_signal_handler);

	char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
	char* parsedArgsPiped[MAXLIST]; 
	int execFlag = 0; 
	init_shell(); 

	while (1) { 
		// print shell line 
		//printDir(); 
		// take input 
		if (takeInput(inputString)) 
			continue; 
		// process 
		execFlag = processString(inputString, 
		parsedArgs, parsedArgsPiped); 

		// execute 
		if (execFlag == 1) 
			execArgs(parsedArgs); 

		if (execFlag == 2) 
			execArgsPiped(parsedArgs, parsedArgsPiped); 
	} 
	return 0; 
} 

