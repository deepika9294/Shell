#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include<setjmp.h>
#include <signal.h>

// struct instn {
// 	char **cmd;
// 	int argcount;
// }

// ************************************************************
char prompt[1024];
char cwd[1024]; 
// char *cmd;

// ************************************************************

void show_prompt() {
	// getting current directory details, an printing prompt accordingly.
	char* user_name = getenv("USER"); 
	if(getcwd(cwd, sizeof(cwd)) != NULL){
        strcpy(prompt, user_name);
		strcat(prompt, "@prompt:");
		strcat(prompt, cwd);
		strcat(prompt, "$ ");
		printf("%s", prompt);
    }
    else{
        perror("Something went wrong, while fetching cwd");
        exit(1);
    }

}

void shell_info() 
{ 
    system("clear");
    printf("\n\n\n\n******************"
        "************************"); 
	printf("\n\n\t-WELCOME TO MY SHELL"); 
    printf("\n\n\n\n*******************"
        "***********************"); 
    printf("\n"); 

	// uncomment below code, if need to clear it the initialisation screen

    sleep(2); 
    system("clear");

} 

int read_input(char* cmd) 
{ 
    char* buffer; 
	
    buffer = readline("");
    // strcpy(cmd, buffer); 
	
    if (strlen(buffer) != 0) { 
        // add_history(buffer); 
        strcpy(cmd, buffer); 
        return 0; 
    } else { 
        return 1; 
    } 
} 



void execute_command(char* parsed_cmd) {

}
int main() {
	int pid;
	int length = 0;
	int argcount = 0;
	char* argv[128];
	char cmd[256];
	char delimeter[] = " ";
	// shell_info();
	while(1) {
			argcount = 0;
			show_prompt();
			if (read_input(cmd)) 
            	continue;
			// read_input(cmd);
			// printf("%s\n",cmd);
			char *ptr = strtok(cmd, delimeter);
			while(ptr != NULL)
			{
				// printf("'%s'\n", ptr);
				argv[argcount++] = ptr;
				ptr= strtok(NULL, delimeter);
			}
			printf("%d", argcount);
			argv[argcount] = NULL;
			
			// for (int i = 0; i < argcount; i++ ){
			// 	printf("%s\n", argv[i]);
			// }
			

			pid = fork();
			if(pid == -1){
            	perror("Error in forking");
				exit(0);

        	}
			if(pid == 0) {
				// printf("%s", argv[2]);
				if (execvp(argv[0], argv) < 0) { 
					printf("\nCould not execute command..\n"); 
				} 
				exit(0);
			} else {
				//printf("in parent %d \n", pid);
				wait(0);
			}
	}
	exit(EXIT_SUCCESS);

	return 0;
}
