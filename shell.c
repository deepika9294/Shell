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
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[0;32;32m"
#define COLOR_NONE "\033[m"


// ******************************GLOBAL VARIABLES******************************
char prompt[1024];		//buffer variable to display a prompt while shell is running
char cwd[1024]; 		//variable to store the current working directory
char home_dir[128];		//variable for hd, required for command "cd"
int is_pipe = 0;		//whether it is a pipe command or not
int is_redirect = 0; 	//whether it is a redirection command or not
static sigjmp_buf env;	//buffer env variable for jumping
static volatile sig_atomic_t jump_active = 0;	
int pid_stop;			//id which is passed when ctrl+z is pressed
int bg[64];				//64 max process can be there in bg
char bg_cmd[64][128];	//commands. can be done in structure too
int bg_count = 0;
pid_t shell_id;			//to store the process id of shell
char t_cmd[128];		
int is_bg_process = 0;


// ************************************************************
/*
*this function takes in array of strings as a command and 
*check whether a particular symbol is present in it or not
*/

int check_symbol(char **cmd, char *symbol) {
	int i = 0;
	int k;
	for(int j = 0; cmd[j] != NULL; j++) {
		// printf(" %s ", cmd[j]);
		if(strcmp(cmd[j], symbol) == 0){
			for(k = j; cmd[k] != NULL; k++) {
				if(cmd[k+1] != NULL)
					cmd[k] = cmd[k+1];
			}
			cmd[k-1] = NULL;
			free(cmd[k]);
			return j;
		}
	}
	return -1;
}
/*
	This function closes the standard input strem and assign fd to the input file,
	to read from the file
*/
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

/*
	This function closes the standard output strem and assign fd to the output file,
	to write to the file
*/

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

/*
	Function to execute Multiple Pipes with redirections as well, 
	and create child processes accordingly and execute the commands.
*/

void create_pipe(char ***c)
{
	int fd[2] = {-1,-1};
	pid_t pid;
	int fdd = 0;				
	int  f = 0;
	int q = 0;
	int i_loc, o_loc;
	char i_file[128], o_file[128];
	
	//DEBUGGING CODE for checking cmd variable

	// for ( int x = 0; c[x] != NULL; x++) {
	// 	for(int y = 0; c[x][y] != NULL; y++) {
	// 		printf("' %s '", c[x][y]);
	// 	}
	// 	printf("\n");
	// }
	// while (*c != NULL) {
	for ( int x = 0; c[x] != NULL; x++) {
		
		pipe(fd);				
		i_loc = check_symbol(c[x], "<");
		if(i_loc != -1) {
			strcpy(i_file, c[x][i_loc]);
		}
		
		o_loc = check_symbol(c[x], ">");
		if(o_loc != -1) {
			strcpy(o_file, c[x][o_loc]);
		}
		
		if ((pid = fork()) == -1) {
			perror("Error in fork");
			return;
		}
		else if (pid == 0) {
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);

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
			fdd = fd[0];
			close(fd[1]);
		}
		free(c[x]);
	}
}

/*
	Function to get the home directory which is used for printing prompt 
	and in executing "cd" command
*/


void get_home_directory() {
	char *temp = getenv("HOME");
	strcpy(home_dir, temp);
	//alternate code, if it fails
	if(home_dir == NULL) {
		perror("Error: Could not access home directory");
		exit(1);
	}

}

/*
	Function to show the user friendly prompt, comprises of
	name+@promt+cwd+$
*/

void show_prompt() {
	// getting current directory details, an printing prompt accordingly.
	char* user_name = getenv("USER"); 
	if(getcwd(cwd, sizeof(cwd)) != NULL){
        strcpy(prompt, user_name);
		strcat(prompt, "@prompt:");
		strcat(prompt, cwd);
		strcat(prompt, "$ ");
		printf(COLOR_GREEN "%s" COLOR_NONE, prompt);
    }
    else{
        perror("Something went wrong, while fetching cwd");
        exit(1);
    }

}

/*WELCOME FUNCTION*/

void shell_info() { 
    system("clear");
    printf("\n\n\n\n******************************************"); 
	printf("\n\n\t-WELCOME TO MY SHELL"); 
	printf("\n\n\t					-Deepika Goyal"); 

    printf("\n\n\n\n******************************************"); 
    printf("\n"); 

	//comment below code, if do not want to clear it the initialisation screen
    sleep(2); 
    system("clear");
} 

/*
	Function to read input from main function using readline library, 
	which helps in autocompleting the commands too. Copies the input in cmd. 
	This is called at every iteration in main()
*/

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

/*
	Typical function to separate a string based on space as a delimeter and forms an array of string.
	Also returns the count of null terminated array
*/

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
		argv[count++] = ptr;
		ptr= strtok(NULL, delimeter);
	}
	argv[count] = NULL;
	return count;
}
/*
	Function to execute a command by creating child.
	(Note, currently this function is not in use, just for backup purpose)
*/

void execute_command(char** argv, int argcount) {

	int pid = fork();
		if(pid == -1){
			perror("Error in forking");
			exit(0);

	}
	if(pid == 0) {
		if (execvp(argv[0], argv) < 0) { 
			printf("\nCould not execute command..\n"); 
		} 
		exit(0);
	} else {
		wait(0);
	}
	return;
}

/*
	This function takes in the cmd and the location where redirection was found and 
	accordingly finds input filename
*/
void get_input_file(char* cmd, int loc, char* input_file) {
	int j = 0;
	for(int i = loc + 2; i < strlen(cmd) && cmd[i]!=' '; i++) {
		input_file[j++] = cmd[i];
	}
	
	return;
}

/*
	This function takes in the cmd and the location where redirection was found and 
	accordingly finds output filename

*/

void get_output_file(char* cmd, int loc, char* output_file) {
	int j = 0;
	// check for more conditions
	for(int i = loc + 2; i < strlen(cmd) && cmd[i]!=' '; i++) {
		output_file[j++] = cmd[i];
	}
	
	return;
}

/*
	This function takes in cmd and a character, 
	if a charater is found then return the location else return -1
*/

int string_compare(char* cmd, char ch) {
	for(int i = 0; i < (strlen(cmd)); i++ ) {
		if(cmd[i] == ch) {
			return i;
		}
	}
	return -1;
}

/*
	General tokenisation , which takes in a string and delimeter and 
	create a null terminated array of strings separated by the delimeter
*/

int tokenisation(char* cmd, char* delimeter, char** redirect_cmd) {
	char *temp_cmd;
	int count = 0;
	// printf("--%s--\n",delimeter);
	temp_cmd = (char*)malloc(sizeof(cmd) + 1);
	strcpy(temp_cmd,cmd);
	// basic parsing, separating a line with space in it.
	char *ptr = strtok(temp_cmd, delimeter);
	while(ptr != NULL)
	{
		// printf("'%s'\n", ptr);
		redirect_cmd[count++] = ptr;
		ptr= strtok(NULL, delimeter);
	}
	return count;
}

/*
	General handler function called by parent process for handling CTRL+C signal, 
	it basically restart the shell/ iteration
*/

void handle_sigint(int signo) {
	if (!jump_active) {
        return;
    }
	if(is_bg_process == 1) {
		// signal(SIGINT, SIG_IGN);
		// while(1);	//bad idea
		return;
	}
	siglongjmp(env, 73);

}

/*
	General handler function called by parent process for handling CTRL+Z signals,
	It first checks for the shell id and if found equal, it returns.
	Else it stores the pid_stop id in the global variable along with cmd and update the count
	bg_count starts with 0 and can have maximum as 64
*/

void handle_sigstp(int signo) {
	if(is_bg_process == 1) {
	// 	siglongjmp(env, 73);
		// while(1);
		return;

	}
	if(pid_stop == shell_id || pid_stop == bg[bg_count-1]) {
		fprintf(stdout, "\n");
		show_prompt();
		return;
		// if(!jump_active) {
        // 	return;
   		// }
		// siglongjmp(env, 73);

	}
	bg[bg_count] = pid_stop;
	strcpy(bg_cmd[bg_count], t_cmd);
	bg_count++;
	fprintf(stdout, "\n[%d]+:	Stopped		Pid: %d\n", bg_count-1, pid_stop);
	if(!jump_active) {
        return;
    }
	siglongjmp(env, 73);

}

/*
*/
void remove_bg(int process) {
	// refill it with 0 and provide error handling
	// for(int i = process; i < bg_count-1; i++ ) {
	// 	strcpy(bg_cmd[process],bg_cmd[process+1]);
	// 	bg[process] = bg[process + 1];
	// }
	// bg_count--;
	strcpy(bg_cmd[process], "");
	bg[process] = -1;

}

/*
	Handle foreground process, takes in the bg_id of the process which needs to be executed
	do some error checking, like background procees exist or not etc, and edit the global bg array
*/
void handle_fg(int process) {
	if(process > bg_count) {
		printf("Background Process doesn't exist\n");
		// return;
		// exit(1);
		siglongjmp(env,73);

	}
	if(bg[process] == -1 ) {
		printf("fg: %d :No such job\n", process);
		siglongjmp(env,73);

		// return;
	}
	// printf("\n%s\n",c);
	printf("\n%s\n",bg_cmd[process]);
	kill(bg[process],SIGCONT);
	remove_bg(process);
	siglongjmp(env,73);
}


void handle_bg(int process) {
	
	if(process > bg_count) {
		printf("Process doesn't exist\n");
		siglongjmp(env,73);

	}
	if(bg[process] == -1 ) {
		printf("fg: %d :No such job\n", process);
		siglongjmp(env,73);

	}
	printf("\n%s\n",bg_cmd[process]);
	kill(bg[process],SIGCONT);
	remove_bg(process);
	siglongjmp(env,73);

}

// ------------------------------------------MAIN DRIVER CODE----------------------------------------------------
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
	shell_id = getpid();
	// pid_stop = shell_id;

	signal(SIGINT, handle_sigint);
	signal(SIGTSTP, handle_sigstp);
	while(1) {
		// shell_id = getpid();
		
		pid_stop = shell_id;
		is_bg_process = 0;

		if (sigsetjmp(env, 1) == 73) {
            printf("\n");
			// show_prompt();
        }
		jump_active = 1;        /* Set the flag */

		argcount = 0;
		show_prompt();
		

		if (read_input(cmd)) 
			continue;
		
		
		strcpy(t_cmd,cmd);	//for bg

		pipe_loc = string_compare(cmd, '|');
		if(pipe_loc != -1) {
			// printf("Enter pipe\n");
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
			// printf("ddd");
			free(A);
			// for()
			continue;
		}
		

		if(pipe_loc == -1) {
			argcount = space_tokenisation(cmd,argv);
			
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
			// printf("--------DEBUGGING---------\n");
			// for (int i = 0; i < argcount; i++ ){
			// 	printf("%s\n", argv[i]);
			// }
			// printf("\nINPUT FILE: %s", input_file);
			// printf("\nOUTPUT FILE: %s\n", output_file);

			// // printf("CMD %s\n", cmd);
			// printf("-----------------\n");
			// execute_command(pid,argv,argcount);

		if(strcmp(argv[0], "fg") == 0) {
			if(bg_count != 0) {
				if(argcount >= 2) {
					handle_fg(atoi(argv[1]));
				}
				handle_fg(bg_count-1);
			}
			else {
				printf("No Processes in background\n");
			}
			
			continue;
		}

		if(strcmp(argv[0], "bg") == 0) {
			is_bg_process = 1;
			if(bg_count != 0) {
				if(argcount >= 2) {
					handle_bg(atoi(argv[1]));
				}
				handle_fg(bg_count-1);
			}
			else {
				printf("No Processes in background\n");
			}
			continue;
		}

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
		pid_stop = pid;

		if(pid == -1){
			perror("Error in forking");
			exit(0);

		}
		if(pid == 0) {
			// printf("%s", argv[2]);
			// pid_stop = pid;
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);

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
			// wait(0);
			waitpid(pid,0, WUNTRACED);
		}
	}}
	exit(EXIT_SUCCESS);
	return 0;
}
