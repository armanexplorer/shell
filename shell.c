// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<dirent.h>
#include<fnmatch.h>
#include<signal.h>
#include<fcntl.h>
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
int command_is_in_process=0;

// Parent SIGINT Handler
void parent_terminate_signal_handler(int signum){
	if (getpid()!=MAIN_PID)
		exit(0);
	if(!command_is_in_process){
		printf("\n\n"); // Move to a new line
	    	rl_on_new_line(); // Regenerate the prompt on a newline
	    	rl_replace_line("", 0); // Clear the previous text
		printf(CYN); // Change color of default prompt text
	    	rl_redisplay(); // Print default prompt text again
	}
	else{
		printf("\n");
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
	char str[300] = CYN "\nplease enter a command> " RESET;
	strcpy(intro_text, str);
} 

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

// Help command builtin 
void openHelp() 
{ 
	puts("\n***WELCOME TO THE SHELL HELP***"
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>lsdir"
		"\n>rename"
		"\n>ash"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>redirect output handling"
		"\n>improper space handling"); 

	return; 
} 


// Function to execute builtin commands 
int ownCmdHandler(char** parsed, int just_check) 
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

	if(just_check)
		return i==NoOfOwnCmds?0:(i+1);

	pid_t p1;
	switch (switchOwnArg) { 
	case 1:  // exit
		printf(MAG "Goodbye!\n\n" RESET); 
		exit(0); 
	case 2:  // cd
		if(parsed[2] != NULL || parsed[1] == NULL){
			fprintf(stderr, RED "illegal number of arguments\n" RESET);
			return 1;
		}
		if(chdir(parsed[1]) == -1){
			if(errno == ENOENT)
				fprintf(stderr, RED "directory '%s' does not exist\n" RESET, parsed[1]);
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


// Function where the system command is executed 
void execArgs(char** parsed) 
{
	// Forking a child 
	pid_t pid = fork(); 

	if (pid == -1) { 
		fprintf(stderr, RED "Failed forking child..\n" RESET); 
		return; 
	} else if (pid == 0) {  // child
		if (execvp(parsed[0], parsed) < 0) { 
			fprintf(stderr, RED "Could not execute command..\n" RESET); 
		} 
		exit(0); 
	} else { 
		// waiting for child to terminate 
		wait(NULL); 
		return; 
	} 
} 

// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe, int is_own_cmd) 
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
		if(dup2(pipefd[1], STDOUT_FILENO) == -1){
			perror(RED "dup 2 falied" RESET);
			exit(0);
		} 
		close(pipefd[1]); 

		if (is_own_cmd){
			ownCmdHandler(parsed, 0);
			exit(0);
		}
		else if (execvp(parsed[0], parsed) < 0) { 
			fprintf(stderr, RED "\nCould not execute command 1.." RESET); 
			exit(0); 
		}

	} else { 
		// Parent executing 
		p2 = fork(); 

		if (p2 < 0) { 
			fprintf(stderr, RED "Could not fork\n" RESET); 
			return; 
		} 

		// Child 2 executing.. 
		// It only needs to read at the read end 
		if (p2 == 0) { 
			close(pipefd[1]); 
			if(dup2(pipefd[0], STDIN_FILENO) == -1){
				perror(RED "dup2 failed" RESET);
				exit(0);
			}
			close(pipefd[0]); 

			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				fprintf(stderr, RED "Could not execute command 2..\n" RESET); 
				exit(0); 
			} 

		} else { 
			// parent executing, waiting for two children 

			// parent must close the pipes before wait
			for(int i=0;i<2;i++)
           			close(pipefd[i]);

			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 


// functoin for finding reidrect
int parseRedirect(char *str, char *target){
	char *temp_str;
	strsep(&str, ">");
	temp_str = strsep(&str, ">");
	if(temp_str != NULL){
		int j=0;
		for(int i=0;i<strlen(temp_str);i++)
			if(temp_str[i] != ' ')
				target[j++] = temp_str[i];
		target[j] = '\0';
	}
	else
		return 0;
	if(strsep(&str, ">") != NULL)
		return -1;
	return 1;
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

int processString(char* str, char** parsed, char** parsedpipe, int *stdout_file_des) 
{ 

	char* strpiped[2]; 
	char target[30];
	int piped = 0; 
	int rdir = 0;

	rdir = parseRedirect(str, target);
	if( rdir == -1){
		fprintf(stderr, RED "invalid use of redirect operator" RESET);
		return -1;
	}
	else if(rdir == 1){
		int out_file_des = open(target, O_WRONLY|O_CREAT, 0600);
		if(out_file_des == -1){
			perror("error in open the file");
			return -1;
		}
		*stdout_file_des = dup(1);
		if(-1 == dup2(out_file_des, 1)){
			perror("error in duplicating target file by stdout file descriptor");
			return -1;
		}
		close(out_file_des);
	}
	piped = parsePipe(str, strpiped); 
	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe); 

	} else { 
		parseSpace(str, parsed); 
	} 

	if (ownCmdHandler(parsed, 1)) 
		return piped;

	return 2 + piped; 
} 

int main() 
{ 
	MAIN_PID = getpid();
	signal(SIGINT, parent_terminate_signal_handler);

	char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
	char* parsedArgsPiped[MAXLIST]; 
	int execFlag = 0; int stdout_file_des=-1; 
	init_shell(); 

	while (1) { 
		command_is_in_process = 0;
		if (takeInput(inputString)) 
			continue; 
		command_is_in_process = 1;

		// process 
		execFlag = processString(inputString, parsedArgs, parsedArgsPiped, &stdout_file_des); 

		// error in syntax
		if (execFlag == -1);

		// this is ownCmd without piped
		if (execFlag == 0)
			ownCmdHandler(parsedArgs, 0);

		// this is ownCmd with piped
		if (execFlag == 1)
			execArgsPiped(parsedArgs, parsedArgsPiped, 1);

		// run in execvp without piped
		if (execFlag == 2) 
			execArgs(parsedArgs); 

		// run in execvp with piped
		if (execFlag == 3) 
			execArgsPiped(parsedArgs, parsedArgsPiped, 0); 

		// if we had redirecting, reset the settings
		if(stdout_file_des != -1){
			if( -1 == dup2(stdout_file_des, 1)){
				perror("fail on assign '1' file descirptor to stdout");
				exit(0);
			}
			close(stdout_file_des);
			stdout_file_des = -1;
		}
	} 
	return 0; 
} 

