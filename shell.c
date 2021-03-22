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
#include <setjmp.h>
#include <signal.h>


// ******************************GLOBAL VARIABLES******************************
char prompt[1024];		//buffer variable to display a prompt while shell is running
char cwd[1024]; 		//variable to store the current working directory
char home_dir[128];		//variable for hd, required for command "cd"
int is_pipe = 0;		//whether it is a pipe command or not
int is_redirect = 0; 	//whether it is a redirection command or not
// static jmp_buf env;
static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;




// ************************************************************
int check_symbol(char **cmd, char *symbol) {
	int i = 0;
	for(int j = 0; cmd[j] != NULL; j++) {
		// printf(" %s ", cmd[j]);
		if(strcmp(cmd[j], symbol) == 0){
			for(int k = j; cmd[k] != NULL; k++) {
				if(cmd[k+1] != NULL)
					cmd[k] = cmd[k+1];
			}
			return j;
		}
	}
	return -1;
}
int input_redirect(char** cmd, char* input_file) {
	// open file and read;
	// close(0);
	int fd = open(input_file, O_RDONLY);
	if(fd == -1) {
		perror("Error in reading input file");
		exit(0);
	}
	dup2(fd, STDIN_FILENO); 
	close(fd);
	return 0;
}

void output_redirect(char** cmd, char* output_file) {
	// close(1);
	int fd = open(output_file, O_WRONLY | O_CREAT, 0666);
	if(fd == -1) {
		perror("Error in writing to output file");
		exit(0);
	}
	dup2(fd, STDOUT_FILENO); 
	close(fd);
	return;
}

void create_pipe(char ***c)
{
	int fd[2] = {-1,-1};
	pid_t pid, fpid;
    //make an array of pids
	int fdd = 0, count=0;				
	count = 0;
	int  f = 0;
	int q = 0;
	int i_loc, o_loc;
	char i_file[128], o_file[128];
	// while(*c != NULL) {
	// 	while(**c != NULL) {
	// 		printf("' %s '", **)
	// 	}

	// }
	// for ( int x = 0; c[x] != NULL; x++) {
	// 	for(int y = 0; c[x][y] != NULL; y++) {
	// 		printf("' %s '", c[x][y]);
	// 	}
	// 	printf("\n");
	// }
	// while (*c != NULL) {
	for ( int x = 0; c[x] != NULL; x++) {
		
		pipe(fd);				
		// printf("%d %d\n", fd[0], fd[1]);
		i_loc = check_symbol(c[x], "<");
		if(i_loc != -1) {
			strcpy(i_file, c[x][i_loc + 1]);
		}
		
		o_loc = check_symbol(c[x], ">");
		if(o_loc != -1) {
			strcpy(o_file, c[x][o_loc+1]);
		}
		
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			signal(SIGINT, SIG_DFL);
			dup2(fdd, STDIN_FILENO);
			
			if (c[x+1] != NULL) {
				dup2(fd[1], STDOUT_FILENO); 
			}
			
			close(fd[0]);
			if(i_loc != -1) {
				input_redirect(c[x],i_file);
			}
			if(o_loc != -1) {
				output_redirect(c[x],o_file);
			}
			if (execvp(c[x][0], c[x]) < 0) { 
				printf("\nCould not execute command..\n");
				exit(1);

			} 
		}
		else {
			wait(NULL);
			// count++;
			fdd = fd[0];
			close(fd[1]);
		}
	}
	// int i = 0, cpid;
	// while(i<count){
	// 	cpid = wait(NULL);
	// 	i++;
	// }
}

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


void execute_command(char** argv, int argcount) {

	int pid = fork();
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
	for(int i = loc + 2; i < strlen(cmd) && cmd[i]!=' '; i++) {
		input_file[j++] = cmd[i];
	}
	
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


int tokenisation(char* cmd, char* delimeter, char** redirect_cmd) {
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
void handle_sigint(int signo) {
	if (!jump_active) {
        return;
    }
	siglongjmp(env, 73);

}

// void pipe_redirection(char** cmd, int )

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
	int pipe_count;



	signal(SIGINT, handle_sigint);
	// shell_info();
	while(1) {
		if (sigsetjmp(env, 1) == 73) {
            printf("\n");
			// show_prompt();

        }
		jump_active = 1;        /* Set the flag */

		argcount = 0;
		show_prompt();
		if (read_input(cmd)) 
			continue;
		

		pipe_loc = string_compare(cmd, '|');
		if(pipe_loc != -1) {
			printf("Enter pipe\n");
			pipe_count = tokenisation(cmd, "|", pipe_cmd);
			char*** A = (char***)malloc((pipe_count+1) * sizeof(char**));

			for(int i = 0; i < pipe_count;i++) {
				argcount1 = space_tokenisation(pipe_cmd[i],argv1);
				A[i] = (char**)malloc((argcount1 +1) * sizeof(char*));
				
				for(int j = 0;j < argcount1; j++) {
					A[i][j] = (char *)malloc(64 * sizeof(char));
					strcpy(A[i][j], argv1[j]);
				}
				A[i][argcount1] = NULL;
			}
			A[pipe_count] = NULL;
			// for(int j = 0; j < 3; j++) {
			// 	printf("' %s ' ", A[0][j]);
			// }
			create_pipe(A);
			printf("ddd");
			continue;
		}
		

		if(pipe_loc == -1) {
			argcount = space_tokenisation(cmd,argv);
			printf("sss");
			input_redirect_loc =string_compare(cmd, '<');
			if(input_redirect_loc != -1) {
				get_input_file(cmd,input_redirect_loc,input_file);
				tokenisation(cmd,"<",redirect_cmd);
				argcount = space_tokenisation(redirect_cmd[0],argv);
			}
			output_redirect_loc = string_compare(cmd, '>');
			if(output_redirect_loc != -1) {
				get_output_file(cmd, output_redirect_loc, output_file);
				tokenisation(cmd,">",redirect_cmd);
				if(input_redirect_loc != -1) {
					tokenisation(cmd, "< >", redirect_cmd);
				}
				argcount = space_tokenisation(redirect_cmd[0],argv);
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
			// signal(SIGINT, handle_sigint);
			signal(SIGINT, SIG_DFL);

			if(input_redirect_loc != -1) {
				input_redirect(argv,input_file);
			}
			if(output_redirect_loc != -1) {
				output_redirect(argv,output_file);
			}
			if (execvp(argv[0], argv) < 0) { 
				printf("\nCould not execute command..\n"); 
			} 
			exit(0);
		} else {
			//printf("in parent %d \n", pid);
			wait(0);
		}
	}}
	exit(EXIT_SUCCESS);
	return 0;
}
