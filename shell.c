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


// ************************************************************
char prompt[1024];
char cwd[1024]; 
char home_dir[128];
int is_pipe = 0;
int is_redirect = 0; 


// ************************************************************



void get_home_directory() {
	char *temp = getenv("HOME");
		strcpy(home_dir, temp);
		// write alternate code, if it fails

        // if (home_dir != NULL) {
        //         printf("Home dir in enviroment");
        //         printf("%s\n", home_dir);
        // }
}

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

// int get_redirection_type(char* cmd) {
// 	/*
// 	*	if normal-0
// 	*	if redirect-1
// 	*	if pipes-2
// 	*	if pipes and redirect-3	(mostly not required)
// 	*	----->for return types
// 	*/


// 	/*
// 	*	1. input-redirect -1
// 	*	2. out-redirect -2
// 		3. input-output - 3
// 	*	3. error-redirect - 4
// 	*	4. for changing structure value
// 	*/
// 	char input_redirect = '<';
// 	char output_redirect = '>';
// 	char output_redirect_append[2] = ">>";
// 	char error_redirect[2] = "2>";

// 	printf("\n pikaboo:%s\n",cmd);
// 	// writng code just for < and > , later modify it
// 	for(int i = 0; i < strlen(cmd); i++) {
// 		if(cmd[i] == input_redirect || cmd[i] == output_redirect) {
// 			if(cmd[i] == input_redirect) {
// 				tc.redirect = 1;
// 			}
// 			else {
// 				tc.redirect = 2;
// 			}
// 			cmd_type = 1;
// 			return i;
// 		}
// 	}
// 	cmd_type = 0;
// 	return -1;

// }
int space_tokenisation(char *cmd, char **argv) {
	char delimeter[] = " ";
	char count = 0;
	char *temp_cmd;

	temp_cmd = (char*)malloc(sizeof(cmd) + 1);
	strcpy(temp_cmd,cmd);


	// basic parsing, separating a line with space in it.
	char *ptr = strtok(temp_cmd, delimeter);
	while(ptr != NULL)
	{
		// printf("'%s'\n", ptr);
		argv[count++] = ptr;
		ptr= strtok(NULL, delimeter);
	}
	argv[count] = NULL;
	return count;
}

// int redirection_tokenisation(char *cmd, char** redirect_cmd) {
	
// 	char delimeter[2];
// 	if(tc.redirect == 1) {
// 		strcpy(delimeter, "<");
// 	}
// 	if(tc.redirect == 2) {
// 		strcpy(delimeter, ">");
// 	}
// 	// char *redirect_cmd[128];
// 	char *temp_cmd;
// 	int count = 0;

// 	temp_cmd = (char*)malloc(sizeof(cmd) + 1);
// 	strcpy(temp_cmd,cmd);


// 	// basic parsing, separating a line with space in it.
// 	char *ptr = strtok(temp_cmd, delimeter);
// 	while(ptr != NULL)
// 	{
// 		// printf("'%s'\n", ptr);
// 		redirect_cmd[count++] = ptr;
// 		ptr= strtok(NULL, delimeter);
// 	}
// 	return count;


// }


void execute_command(int pid ,char** argv, int argcount) {

	if (*argv[0] == '\0') {
		// continue;
		return;
	}


	/* Check if string equals "exit"*/
	if (!(strcmp(argv[0] , "exit"))) {
		printf("Exiting....\n");
		// break;
		exit(EXIT_SUCCESS);

	}

	/* */
	if(strcmp(argv[0], "cd") == 0) {    
		// get home directory

		if (argcount == 1) {
			get_home_directory();	
			if(chdir(home_dir) == -1) {
				perror("cd");
			}
			// printf(" hd %s", home_dir);

		}
		else if(chdir(argv[1]) == -1) {                                            
			perror("cd");                                                   
		}          
		// continue;    
		return;                                                       
	}


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
	return;

}
void get_input_file(char* cmd, int loc, char* input_file) {
	int j = 0;
	// for(int i = 0; i < loc -1; i++) {
	for(int i = loc + 2; i < strlen(cmd) && cmd[i]!=' '; i++) {

		input_file[j++] = cmd[i];
	}
	// for (int i = 0; i < j ;i++ ){
	// 	cmd[loc-1] = cmd[j+5];
	// }
	// printf("%s", cmd);
	return;
}
void get_output_file(char* cmd, int loc, char* output_file) {
	int j = 0;
	// check for more conditions
	for(int i = loc + 2; i < strlen(cmd) && cmd[i]!=' '; i++) {
		output_file[j++] = cmd[i];
	}
	
	return;
}

int string_compare(char* cmd, char ch) {
	for(int i = 0; i < (strlen(cmd)); i++ ) {
		if(cmd[i] == ch) {
			return i;
		}
	}
	return -1;
}

void input_redirect(char** cmd, char* input_file) {
	// open file and read;
	close(0);
	int fd = open(input_file, O_RDONLY);
	if(fd == -1) {
		perror("Error in reading input file");
		exit(0);
	}
}

void output_redirect(char** cmd, char* output_file) {
	close(1);
	int fd = open(output_file, O_WRONLY | O_CREAT, 0666);
	// printf("\nKEya: %d", fd);

	if(fd == -1) {
		perror("Error in writing to output file");
		exit(0);
	}
	return;
}

int redirection_tokenisation(char* cmd, char* delimeter, char** redirect_cmd) {
	char *temp_cmd;
	int count = 0;
	printf("--%s--\n",delimeter);
	temp_cmd = (char*)malloc(sizeof(cmd) + 1);
	strcpy(temp_cmd,cmd);
	// basic parsing, separating a line with space in it.
	char *ptr = strtok(temp_cmd, delimeter);
	while(ptr != NULL)
	{
		printf("'%s'\n", ptr);
		redirect_cmd[count++] = ptr;
		ptr= strtok(NULL, delimeter);
	}
	return count;
}

int main() {
	int pid;
	int length = 0;
	int argcount = 0;
	char* argv[128];
	char cmd[256];
	char delimeter[] = " ";
	char s[100];
	char *temp_cmd;
	char *redirect_cmd[128];
	int redirect_count;
	int input_redirect_loc;
	int output_redirect_loc;
	int redirect_argcount = 0 ; //might change to argcount
	char input_file[64];
	char output_file[64];
	char *rc;
	int fd;

	// initialising types to be nilll

	// struct type_command tc;
	// shell_info();
	while(1) {
			argcount = 0;
			show_prompt();
			if (read_input(cmd)) 
            	continue;
			
			// check if redirection is present.
			if(strchr(cmd, '|')) {
				printf("Not implemented");
				continue;
			}

			argcount = space_tokenisation(cmd,argv);

			input_redirect_loc =string_compare(cmd, '<');
			if(input_redirect_loc != -1) {
				get_input_file(cmd,input_redirect_loc,input_file);
				printf("input:%s",input_file);
				redirection_tokenisation(cmd,"<",redirect_cmd);
				// if it exist
				argcount = space_tokenisation(redirect_cmd[0],argv);
				

			}
			output_redirect_loc = string_compare(cmd, '>');
			if(output_redirect_loc != -1) {
				get_output_file(cmd, output_redirect_loc, output_file);
				redirection_tokenisation(cmd,">",redirect_cmd);
				printf("REdirect: %s\n",redirect_cmd[0]);
				argcount = space_tokenisation(redirect_cmd[0],argv);
			// 	printf("-----------------\n");
			// for (int i = 0; i < argcount; i++ ){
			// 	printf("%s---\n", argv[i]);
			// }
			// // printf("CMD %s\n", cmd);
			// printf("-----------------\n");
				// printf("%")
				// execute_command(pid,argv,argcount);
			}

			// input_redirect(cmd,input_file);

			


			/*Debugging purpose*/
			printf("-----------------\n");
			for (int i = 0; i < argcount; i++ ){
				printf("%s\n", argv[i]);
			}
			// printf("CMD %s\n", cmd);
			printf("-----------------\n");
			// execute_command(pid,argv,argcount);
			if (*argv[0] == '\0') {
				continue;
				// return;
			}

			/* Check if string equals "exit"*/
			if (!(strcmp(argv[0] , "exit"))) {
				printf("Exiting....\n");
				break;
				// exit(EXIT_SUCCESS);

			}

			/* */
			if(strcmp(argv[0], "cd") == 0) {    
				// get home directory

				if (argcount == 1) {
					get_home_directory();	
					if(chdir(home_dir) == -1) {
						perror("cd");
					}
					// printf(" hd %s", home_dir);

				}
				else if(chdir(argv[1]) == -1) {                                            
					perror("cd");                                                   
				}          
				continue;    
				// return;                                                       
			}


			pid = fork();
			if(pid == -1){
				perror("Error in forking");
				exit(0);

			}
			if(pid == 0) {
				// printf("%s", argv[2]);
				if(input_redirect_loc != -1) {
					input_redirect(argv,input_file);
				}
				if(output_redirect_loc != -1) {
					output_redirect(argv,output_file);
				}
				printf("\argv:%s",argv[0]);
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
