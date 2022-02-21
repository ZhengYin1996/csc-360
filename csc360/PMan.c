#include <unistd.h>            
#include <string.h>            
#include <ctype.h>             
#include <stdio.h>             
#include <stdlib.h>            
#include <sys/types.h>              
#include <signal.h>            
#include <readline/readline.h>
#include <stdlib.h>
#include <sys/wait.h>
#define INPUT_SIZE 128
#define LSH_TOK_DELIM " \t\r\n\a"
int lineSize = 0;
int bg_count = 0;
typedef struct node{
	pid_t pid;
	char* path;
	char *stat;
	struct node* next;
	
}node;
node *process1 = NULL;
void addToList(pid_t pid, char* path, int sta)
{
	node* process = (node*)malloc(sizeof(node));
	node->pid = pid;
	node->path = path;
	node->stats = stal
	node->next = NULL;
	if(process1==NULL)
	{
		process1=node;
	}
	else
	{
		node* temp = process1;
		while(temp->next!=NULL)
		{
			temp = temp->next;
		}
		temp->next = process;
	}
	bg_count++;
}

int checkProcess(pid_t pid)
{
	node* temp = process1;
	while(temp!=NULL)
	{
		if (temp->pid ==pid)
			return 1;
		temp = temp->next;
	}
	return 0;
}
void bg(char** input)
{
	pid_t pid = fork();
	int sta =1;
	if(pid == 0) //child
	{
		char* command = input[1];
		execvp(command, &input[1]);
		printf("Error: failed to execute command %s\n", command);
		exit(1);
	}
	else if(pid>0) //parent
	{
		char* path = (char*)malloc(sizeof(char)*INPUT_SIZE);
		realpath(input[1],path);
		addToList(pid,path,sta);
	}
	else
	{
		printf("Error: fail to fork\n");
	}
	
}
void bgkill(pid_t pid)
{
	if(checkProcess(pid)==0)
	{
		printf("Error: invalid pid\n");
		return;
	}
	int error - kill(pid, SIGTERM);
	if(!error)
	{
		sleep(1);
	}
	else
	{
		printf("Error: failed to execute bgkill\n");
	}
}
void bgstop(pid_t pid)
{
	if(checkProcess(pid)==0)
	{
		printf("Error: invalid pid\n");
		return;
	}
	int error = kill(pid, SIGSTOP);
	if (!error) 
	{
		sleep(1);
	} 
	else {
		printf("Error: failed to execute bgstop\n");
	}
	
}
void bgstart(pid_t pid)
{
	if(checkProcess(pid)==0)
	{
		printf("Error: invalid pid\n");
		return;
	}
	int error = kill(pid, SIGCONT);
	if (!error) 
	{
		sleep(1);
	} 
	else 
	{
		printf("Error: failed to execute bgstart\n");
	}
	
}
void bglist()
{
	
	process* temp = process1；
	while(temp!=NULL)
	{
		printf("%d  %s\n",temp->pid, temp->path);
		temp = temp->next;
	}
	printf("Total background jobs: %d\n", bg_count);
}
void pstat(pid_t pid)
{
	
	
}
void identify(char** command)
{
	char* cmd = command[0];
	if(strcmp(cmd, "bg")==0)
	{
		bg(command);
	}
	else if(strcmp(cmd,"bglist")==0)
	{
		bglist();
	}
	else if(strcmp(cmd,"bgkill")==0)
	{
		int pid = atoi(command[1]);
		if(pid==0)
		{
			printf("invalid input on pid");
			exit(1);
		}
		else
		{
			bgkill(pid);
		}
	}
	else if(strcmp(cmd,"bgstop")==0)
	{
		int pid = atoi(command[1]);
		if(pid==0)
		{
			printf("invalid input on pid");
			exit(1);
		}
		else
		{
			bgstop(pid);
		}
	}
	else if(strcmp(cmd,"bgstart")==0)
	{
		int pid = atoi(command[1]);
		if(pid==0)
		{
			printf("invalid input on pid");
			exit(1);
		}
		else
		{
			bgstart(pid);
		}
	}
	else if(strcmp(cmd,"pstat")==0)
	{
		int pid = atoi(command[1]);
		if(pid==0)
		{
			printf("invalid input on pid");
			exit(1);
		}
		else
		{
			pstat(pid);
		}
	}
	else
	{
		printf(" command not found");
	}
	
}
char **lsh_split_line(char* line)
{
	lineSize = 0;
	int bufSize = INPUT_SIZE, position=0;
	char** tokens= malloc(bufSize*sizeof(char*));
	char* token;
	token = strtok(line,LSH_TOK_DELIM);
	while(token!=NULL)
	{
		tokens[position] = token;
		position++;
		if(position>= bufSize)
		{
			bufSize+=INPUT_SIZE;
			tokens = realloc(tokens,bufSize*sizeof(char*));
			if (!tokens) 
			{
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL,LSH_TOK_DELIM);
	}
	tokens[position]=NULL;
	lineSize = position-1;
	return tokens;
}
char *input()
{
	char* header = "PMan: >";
	char* line = readline(header);
	printf("read complate");
	free(header);
	return line;
}
int main(void){
	while(1){
		char** cmd;
		char* line = input()
		if (line == NULL) continue;
		cmd = lsh_split_line(line);
		identify(cmd);
	}	
}
