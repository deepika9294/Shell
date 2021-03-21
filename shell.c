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


// ******************************GLOBAL VARIABLES******************************
char prompt[1024];		//buffer variable to display a prompt while shell is running
char cwd[1024]; 		//variable to store the current working directory
char home_dir[128];		//variable for hd, required for command "cd"
int is_pipe = 0;		//whether it is a pipe command or not
int is_redirect = 0; 	//whether it is a redirection command or not


// ************************************************************



void get_home_directory() {
	char *temp = getenv("HOME");
	strcpy(home_dir, temp);
	//alternate code, if it fails
	if(home_dir == NULL) {
		perror("Error: Could not access home directory");
		exit(1);
	}

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

void shell_info() { 
    system("clear");
    printf("\n\n\n\n******************************************"); 
	printf("\n\n\t-WELCOME TO MY SHELL"); 
    printf("\n\n\n\n******************************************"); 
    printf("\n"); 

	//comment below code, if do not want to clear it the initialisation screen
    sleep(2); 
    system("clear");
} 

int read_input(char* cmd) { 
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
	if(pid == -1) {
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

int redirection_string_compare(char* cmd, char ch) {
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

void create_pipe(char **argv1, char **argv2) {
	int fds[2];	//dual pipe
	pid_t p1, p2;
	if(pipe(fds) < 0) {
		perror("\n Error in initialising the pipe");
		exit(1);
	}
	// creating another process by fork
	p1 = fork();
	if (p1 < 0) {
		perror("\nError in forking");
		exit(1);
	}
	if(p1 == 0) {
		// So we need to close the read end and write to write end
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);

		if (execvp(argv1[0], argv1) < 0) { 
			printf("\nCould not execute command..\n"); 
		} 
	}
	else {
		p2 = fork();
		if (p2 < 0) {
			perror("\nError in forking");
			exit(1);
		}
		// so now we nead to close write end and read it from the read end
		if(p2 == 0) {
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
			if (execvp(argv2[0], argv2) < 0) { 
				printf("\nCould not execute command..\n"); 
			} 
		}
		else {
			for (int i = 0; i < 2; i++) {
				wait(NULL);
			}
		}
	}
}

int main() {
	int pid;
	int length = 0;
	int argcount = 0;
	int argcount1 = 0;
	int argcount2 = 0;
	char* argv[128];
	char* argv1[128];
	char* argv2[128];

	char cmd[256];
	char delimeter[] = " ";
	char s[100];
	char *temp_cmd;
	char *redirect_cmd[128];
	char *pipe_cmd[128];
	int redirect_count;
	int input_redirect_loc;
	int output_redirect_loc;
	int redirect_argcount = 0 ; //might change to argcount
	char input_file[64];
	char output_file[64];
	char *rc;
	int fd;
	int pipe_loc;
	// int pid[2];
	// initialising types to be nilll

	// struct type_command tc;
	// shell_info();
	while(1) {
		argcount = 0;
		show_prompt();
		if (read_input(cmd)) 
			continue;
		
		// check if redirection is present.
		// if(strchr(cmd, '|')) {
		// 	printf("Not implemented");
		// 	continue;
		// }

		argcount = space_tokenisation(cmd,argv);

		input_redirect_loc =redirection_string_compare(cmd, '<');
		if(input_redirect_loc != -1) {
			get_input_file(cmd,input_redirect_loc,input_file);
			printf("input:%s",input_file);
			redirection_tokenisation(cmd,"<",redirect_cmd);
			// if it exist
			argcount = space_tokenisation(redirect_cmd[0],argv);
			

		}
		output_redirect_loc = redirection_string_compare(cmd, '>');
		if(output_redirect_loc != -1) {
			get_output_file(cmd, output_redirect_loc, output_file);
			redirection_tokenisation(cmd,">",redirect_cmd);
			printf("Redirect: %s\n",redirect_cmd[0]);
			if(input_redirect_loc != -1) {
				redirection_tokenisation(cmd, "< >", redirect_cmd);
			}
			// printf("REdirect: %s\n",redirect_cmd[0]);
			// printf("REdirect: %s\n",redirect_cmd[1]);


			argcount = space_tokenisation(redirect_cmd[0],argv);
		
		}
		pipe_loc = redirection_string_compare(cmd, '|');
		if(pipe_loc != -1) {
			redirection_tokenisation(cmd, "|", pipe_cmd);
			argcount1 = space_tokenisation(pipe_cmd[0],argv1);
			argcount2 = space_tokenisation(pipe_cmd[1],argv2);

			printf("PIPE: %s\n",pipe_cmd[0]);
		}
		
		

		/*Debugging purpose*/
		printf("--------DEBUGGING---------\n");
		for (int i = 0; i < argcount; i++ ){
			printf("%s\n", argv[i]);
		}
		printf("\nINPUT FILE: %s", input_file);
		printf("\nOUTPUT FILE: %s\n", output_file);

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
			if(pipe_loc != -1) {
				printf("Enter pipe");
				create_pipe(argv1,argv2);
			}
			if(pipe_loc == -1) {
				if (execvp(argv[0], argv) < 0) { 
					printf("\nCould not execute command..\n"); 
				} 
			}
			exit(0);
		} else {
			//printf("in parent %d \n", pid);
			wait(0);
		}
	}
	// exit(EXIT_SUCCESS);
	return 0;
}
