#include <stdio.h>   // to include printf
#include <signal.h>  // to include struct sigaction handler
#include <unistd.h>  // to include the system function 'execvp'
#include <stdlib.h>  // to include exit()
#include <sys/types.h>  // to include pid_t
#include <sys/wait.h>   // to include wait()
#include <string.h>  // to include strcpy
#define BUFFERSIZE 50
#define MAXLINE 80
#define true 1
#define false 0
 
char buffer[BUFFERSIZE];
char historyCommands[10][81];
int currentCommandIndex = -1; // where to storage the current command
 
void printHis () {
    int i = 0;
    printf("\n");
    for (i = 0 ; i < 10 ; ++i) {
	printf("%d : %s", i, historyCommands[i]);
    }
}
 
void handle_SIGINTO() {
    write(STDOUT_FILENO, buffer, strlen(buffer));    
    printHis();
} 
 
void addHistory(char *inputBuffer) {
    //add history into the message queue
    currentCommandIndex = (currentCommandIndex + 1) % 10;
    strcpy(historyCommands[currentCommandIndex], inputBuffer);    
}
 
char* checkHis (char ch, int type) {
    if (type == 3) {
        int i = 0;
        for (i = currentCommandIndex ; i >= 0 ; --i) {
    	    if (historyCommands[i][0] == ch) {
			return historyCommands[i];
	    }	
	}
		for (i = 9 ; i >= currentCommandIndex ; --i) {
	    	    if (historyCommands[i][0] == ch) {
				return historyCommands[i];
		    }	
		}   
    }
    return NULL;
}
 
void setup(char inputBuffer[], char *args[],int *background) {
	int i = 0;
	int length = -1;  // storage the length of the command
	length = read(STDIN_FILENO, inputBuffer, MAXLINE);
	inputBuffer[length] = '\0';  // read method don't add '\0' automatically
	if (length == 0) {
		exit(0);		 
	}
	else if (length == -1) { 
		args[0] = NULL;  
		return;  // caught nothing, return
	}
	int currParamStartIndex = -1;  // to storage the current parameters'
	int currPos = 0;  // args[currentPosition]
	int copyFlag = false;  // did not call r or 'r x' 	
	char* pCommand = NULL;
	if (inputBuffer[0] == 'r') {
	    // condition 3 : 'r\n' get the most recent one
            if (inputBuffer[1] == '\n') {
            	if (currentCommandIndex == -1) {
                    printf("No valid commands\n");	
		    		return;
                }	
		else {	
		    strcpy(inputBuffer, historyCommands[currentCommandIndex]); 
		    copyFlag = true;
		}
            }    
            else if (inputBuffer[1] == ' ' && inputBuffer[2] != ' ') {
	         if ((pCommand = checkHis(inputBuffer[2], 3)) == NULL) { 
                    printf("No recent valid commands begin with %c\n", inputBuffer[2]);	
		     		return;
                 }
		 else {
                strcpy(inputBuffer, pCommand);                   
                copyFlag = true; 
        	}
	    } 
	}
	if (!copyFlag) {
	    addHistory(inputBuffer);
	}
	else {
            length = strlen(inputBuffer);
	    addHistory(inputBuffer);
        }
	for (i = 0 ; i < length ; ++i) {
		if (inputBuffer[i] == '&') {
			*background = 1;
			inputBuffer[i] = '\0';
		}
		else if (inputBuffer[i] == ' ' || inputBuffer[i] == '\t') {
			if (currParamStartIndex != -1) {
				args[currPos++] = &inputBuffer[currParamStartIndex];
				inputBuffer[i] = '\0';
				currParamStartIndex = -1;
			}
		}
		else if (inputBuffer[i] == '\n') {  // end of the current command
			if (currParamStartIndex != -1) {
				args[currPos++] = &inputBuffer[currParamStartIndex];
				inputBuffer[i] = '\0';
				args[currPos] = NULL;
				currParamStartIndex = -1;
			}
		}
		else {  // storage the valid phrase of the order
			if (currParamStartIndex == -1) {
				currParamStartIndex = i;
			}
		}
	}
	args[currPos] = NULL;  // command length > MAX_LINE
}
 
int main() {
        int i = 0;
	for (i = 0 ; i < 10 ; i++) {
	    historyCommands[i][0] = '\0'; // Initailize the command Array
	}		
	// create signal handler
	struct sigaction handler;
	handler.sa_handler = handle_SIGINTO;
	sigaction(SIGINT, &handler, NULL);
        		
	char inputBuffer[MAXLINE+1];  // maximum for 80 characters
	int background = 0; 
	char* args[MAXLINE/2 + 1];
	while (true) {
		background = 0; // default value to call wait(0)
		printf("COMMAND->");
		fflush(stdout);  // print the order to the screen
		setup(inputBuffer, args, &background);
		pid_t pid = fork();
		if (pid == -1) {
			perror("fork");
			exit(1);  // exit the process by accident
		}
		if (pid == 0) {  // child code
			execvp(args[0], args);
			exit(0);  // exit the process simoutaneously
		}
		if (background == 0) {
			wait(0); 
		}
	}
	return 0;
}
