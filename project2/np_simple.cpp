#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h> //getenv, setenv
#include <unistd.h>
#include<sys/wait.h>
#include <fstream>
#include <fcntl.h>
#include <vector>
#include <sys/types.h>
using namespace std;

#define MAXC 10
#define nu 10000
int pipe1[nu][2] = {0};
string input_str;
vector<int> glo, which_pipe;
int pipe_counter = 0;
int addrlen = 0;
int client_sock[nu] = {0};
struct sockaddr_in serverInfo,clientInfo;

int passivesock(int port){
	int sockfd = 0;
	sockfd = socket(AF_INET , SOCK_STREAM , 0);

	if (sockfd == -1){
		cerr << "Fail to create a socket." << endl;;
	}
	addrlen = sizeof(clientInfo);
	bzero(&serverInfo,sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = htons(port);
	bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	listen(sockfd,5);
	return sockfd;
		
}

void redirection(char* (*command)[nu][MAXC+1], int where_implement){
	int newpid;
	newpid = fork();
	if (newpid == -1){
		cerr << "error" << endl;
	}
	else if (newpid == 0){
		if (where_implement == -1){
			int fd1 = open(*(*command)[1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
			close(1);
			int fd2 = dup(fd1);
			int er = execvp(*(*command)[0],(*command)[0]);
			if (er == -1){
				cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
				exit(1);
			}
			close(fd1);
			close(fd2);
		}
		else{
			close(0);
			dup(pipe1[which_pipe[where_implement]][0]);
			close(pipe1[which_pipe[where_implement]][1]);
			int fd1 = open(*(*command)[1], O_CREAT|O_WRONLY, 0666);
			close(1);
			int fd2 = dup(fd1);
			int er = execvp(*(*command)[0],(*command)[0]);
			if (er == -1){
				cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
				exit(1);
			}
			close(pipe1[which_pipe[where_implement]][0]);
			close(fd1);
			close(fd2);
			exit(0);		
		}

	}
	else{
		if (where_implement != -1){
			close(pipe1[which_pipe[where_implement]][0]);
			close(pipe1[which_pipe[where_implement]][1]);			
		}
		int wait_rv = wait(NULL);
	}
}
void sig_handler(int sig){

        int retval;
        retval = wait(NULL);
}
void simple_com(char* (*command)[nu][MAXC+1], int where_implement){
	int newpid;
	newpid = fork();
	if (newpid == -1){
		cerr << "error" << endl;
	}
	else if (newpid == 0){
		
		if (where_implement == -1){
			int er = execvp(*(*command)[0],(*command)[0]);
			if (er == -1){
				cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
				exit(0);
			}		
		}
		else{
			close(0);
			dup(pipe1[which_pipe[where_implement]][0]);
			close(pipe1[which_pipe[where_implement]][1]);
			int er = execvp(*(*command)[0],(*command)[0]);
			if (er == -1){
				cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
				exit(1);
			}
			close(pipe1[which_pipe[where_implement]][0]);
			exit(0);		
		}
	}
	else{
		if (where_implement != -1){
			close(pipe1[which_pipe[where_implement]][0]);
			close(pipe1[which_pipe[where_implement]][1]);			
		}

		int wait_rv = wait(NULL);
	}
}


void mypipe(char* (*command)[nu][MAXC+1], int &pipe_counter, int pipe_number, int where_implement){
	for(int i = pipe_counter; i < pipe_counter + pipe_number + 1; i++){
		if (i < pipe_counter + pipe_number){
			int ret = pipe(pipe1[i]);
			if (ret == -1){
				cerr << "pipe error!!" << endl;
				exit(1);
			}
		}

		int newpid;
		signal(SIGCHLD,sig_handler);
		newpid = fork();
		if (newpid == -1){
			cerr << "error" << endl;
		}
		else if (newpid == 0){
			if (i == pipe_counter){ //第一個child process
				if (where_implement == -1){	
					close(1);
					dup(pipe1[pipe_counter][1]);
					if (input_str.find("!") != string::npos){//指令直接接!
						close(2);
						dup(pipe1[pipe_counter][1]);
					}/*
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == pipe_counter){
							close(pipe1[j][0]);
						}
						else{
							close(pipe1[j][0]);
							close(pipe1[j][1]);
						}
					}*/
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(pipe1[pipe_counter][1]);
				}
				else{
					close(0);
					dup(pipe1[which_pipe[where_implement]][0]);
					close(pipe1[which_pipe[where_implement]][1]);
					close(1);
					dup(pipe1[pipe_counter][1]);
					if (input_str.find("!") != string::npos){//指令直接接!
						close(2);
						dup(pipe1[pipe_counter][1]);
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == pipe_counter){
							close(pipe1[j][0]);
						}
						else{
							close(pipe1[j][0]);
							close(pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(pipe1[pipe_counter][1]);
					close(pipe1[which_pipe[where_implement]][0]);

				}

			}
			else if (i == pipe_counter + pipe_number){ //最後一個child process
				close(0);
				dup(pipe1[i-1][0]);
				//處理全部的pipe
				for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
					if (j == pipe_counter + pipe_number - 1){//最後一個pipe
						close(pipe1[j][1]);
					}
					else{
						close(pipe1[j][0]);
						close(pipe1[j][1]);
					}
				}
				if (input_str.find(">") != string::npos){
					int fd1 = open(*(*command)[i-pipe_counter+1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
					close(1);
					int fd2 = dup(fd1);
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(fd1);
					close(fd2);
				}
				else{
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
				}
				close(pipe1[i-1][0]);
			}
			else{
				close(0);
				dup(pipe1[i-1][0]);
				close(1);
				dup(pipe1[i][1]);
				if (i == pipe_counter + pipe_number - 1){
					if (input_str.find("!") != string::npos){
						close(2);
						dup(pipe1[i][1]);
					}
				}
				for (int j = pipe_counter; j <= i; j++){
					if (j == i-1){//process 的前一個pipe
						close(pipe1[j][1]);
					}
					else if (j == i){//process後的pipe
						close(pipe1[j][0]);
					}
					else{//其他pipe
						close(pipe1[j][0]);
						close(pipe1[j][1]);
					}
				}
/*
				for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
					if (j == i-1){//process 的前一個pipe
						close(pipe1[j][1]);
					}
					else if (j == i){//process後的pipe
						close(pipe1[j][0]);
					}
					else{//其他pipe
						close(pipe1[j][0]);
						close(pipe1[j][1]);
					}
				}*/
				int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
                                if (er == -1){
                                        cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
                                        exit(1);
                                }
				close(pipe1[i-1][0]);
				close(pipe1[i][1]);
			}	
		}
		else{
	
			if (where_implement != -1){
				close(pipe1[which_pipe[where_implement]][0]);
				close(pipe1[which_pipe[where_implement]][1]);			
			}
			if (i == pipe_counter + pipe_number){
				for (int i = pipe_counter; i < pipe_counter + pipe_number; i++){
					close(pipe1[i][0]);
					close(pipe1[i][1]);
				}
				int wait_rv = wait(NULL);
			}
			
			else{
				int wait_rv = waitpid(newpid, NULL, WNOHANG);
				signal(SIGCHLD,sig_handler);
			}	

		}
	}
}
void numberedpipe(char* (*command)[nu][MAXC+1], int &pipe_counter, int pipe_number, int flag, int where_implement){
	if (flag == -1){
		for(int i = pipe_counter; i < pipe_counter + pipe_number; i++){
			int ret = pipe(pipe1[i]);
			if (ret == -1){
				cerr << "pipe error!!" << endl;
				exit(1);
			}
		}
		for(int i = pipe_counter; i < pipe_counter + pipe_number; i++){
			int newpid;
			newpid = fork();
			if (newpid == -1){
				cerr << "error" << endl;
			}
			else if (newpid == 0){
				if (i == pipe_counter){ //第一個child process
					if (where_implement == -1){	
						close(1);
						dup(pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[pipe_counter][1]);
					}
					else{
						close(0);
						dup(pipe1[which_pipe[where_implement]][0]);
						close(pipe1[which_pipe[where_implement]][1]);
						close(1);
						dup(pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[pipe_counter][1]);
						close(pipe1[which_pipe[where_implement]][0]);

					}
				}
				else{
					close(0);
					dup(pipe1[i-1][0]);
					close(1);
					dup(pipe1[i][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == i-1){//process 的前一個pipe
							close(pipe1[j][1]);
						}
						else if (j == i){//process後的pipe
							close(pipe1[j][0]);
						}
						else{//其他pipe
							close(pipe1[j][0]);
							close(pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(pipe1[i-1][0]);
					close(pipe1[i][1]);
				}	
			}
			else{

				if (where_implement != -1){
					close(pipe1[which_pipe[where_implement]][0]);
					close(pipe1[which_pipe[where_implement]][1]);			
				}
					
				if (i == pipe_counter + pipe_number - 1){
					for (int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
						close(pipe1[i][0]);
						close(pipe1[i][1]);
					}
				}

				int wait_rv = waitpid(newpid,NULL,WNOHANG);
				signal(SIGCHLD,sig_handler);

			}
		}//for
	}//flag == -1
	else{
		for(int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
			int ret = pipe(pipe1[i]);
			if (ret == -1){
				cerr << "pipe error!!" << endl;
				exit(1);
			}
		}
		for(int i = pipe_counter; i < pipe_counter + pipe_number; i++){
			int newpid;
			newpid = fork();
			if (newpid == -1){
				cerr << "error" << endl;
			}
			else if (newpid == 0){
				if (i == pipe_counter && pipe_number > 1){ //第一個child process
					if (where_implement == -1){
						close(1);
						dup(pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[pipe_counter][1]);
					}
					else{
						close(0);
						dup(pipe1[which_pipe[where_implement]][0]);
						close(pipe1[which_pipe[where_implement]][1]);
						close(1);
						dup(pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[pipe_counter][1]);
						close(pipe1[which_pipe[where_implement]][0]);

					}	

				}
				else if (i == pipe_counter && pipe_number == 1){ //第一個child process
					if (where_implement == -1){
						close(1);
						dup(pipe1[which_pipe[flag]][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[which_pipe[flag]][1]);
					}
					else{
						close(0);
						dup(pipe1[which_pipe[where_implement]][0]);
						close(pipe1[which_pipe[where_implement]][1]);
						close(1);
						dup(pipe1[which_pipe[flag]][1]);
						close(pipe1[which_pipe[flag]][0]);

						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number - 1; j++){
							if (j == pipe_counter){
								close(pipe1[j][0]);
							}
							else{
								close(pipe1[j][0]);
								close(pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(pipe1[which_pipe[flag]][1]);
						close(pipe1[which_pipe[where_implement]][0]);


					}	
				}
				else if (i == pipe_counter + pipe_number - 1 && pipe_number > 1){
					close(0);
					dup(pipe1[i-1][0]);
					close(1);
					dup(pipe1[which_pipe[flag]][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number - 1; j++){
						if (j == i-1){//process 的前一個pipe
							close(pipe1[j][1]);
						}
						else{//其他pipe
							close(pipe1[j][0]);
							close(pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(pipe1[i-1][0]);
					close(pipe1[which_pipe[flag]][1]);
					
				}
				else{
					close(0);
					dup(pipe1[i-1][0]);
					close(1);
					dup(pipe1[i][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == i-1){//process 的前一個pipe
							close(pipe1[j][1]);
						}
						else if (j == i){//process後的pipe
							close(pipe1[j][0]);
						}
						else{//其他pipe
							close(pipe1[j][0]);
							close(pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(pipe1[i-1][0]);
					close(pipe1[i][1]);
				}	
			}
			else{
	
				if (where_implement != -1){
					close(pipe1[which_pipe[where_implement]][0]);
					close(pipe1[which_pipe[where_implement]][1]);			
				}

				if (i == pipe_counter + pipe_number - 1){
					for (int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
						close(pipe1[i][0]);
						close(pipe1[i][1]);
					}
				}

				int wait_rv = waitpid(newpid,NULL,WNOHANG);
				/*if (i == pipe_counter + pipe_number - 1){
					int wait_rv = wait(NULL);
				}
				else{
					int wait_rv = waitpid(newpid,NULL,WNOHANG);
				}*/

				signal(SIGCHLD,sig_handler);
			}
		}//for
	}//else
}


int classfy(string input){
	int symbol = 0;

	if (input.find("|") != string::npos || input.find("!") != string::npos){
		symbol = 2;
	}
	else if (input.find(">") != string::npos){
		symbol = 1;
	}
	return symbol;
}
int count_pipe(string input_str){
	int count = 0;
	for(int i = 0; i < input_str.length(); i++){
		if (input_str[i] == '|' || input_str[i] == '!'){
			count++;
		}
	}
	return count;
}

int numberedpipe_num(char* (*arglist)[MAXC+1], int temp_pn, int pipe_number){
	string pipe_with_number((*arglist)[temp_pn-2]);
	int minus_pipe = 0;
	int numpiped_num = 0;
	int flag = -1;
	if (pipe_with_number.find("|") != string::npos){
		char *ab = strtok((*arglist)[temp_pn-2], "|");
		numpiped_num = strtol(ab, NULL, 10);
		for (int i = 0; i < glo.size(); i++){
			if (numpiped_num == glo[i]){
				flag = i;
				break;
			}
		}
		glo.push_back(numpiped_num);
		if (flag != -1){
			which_pipe.push_back(which_pipe[flag]);
			minus_pipe++;
		}
		else{
			which_pipe.push_back(pipe_counter + pipe_number - 1);
		}
	}
	return minus_pipe, numpiped_num;
}
void np(int client_sock){
	char input[nu];
	setenv("PATH","bin:.",1);
	while(1){
		write(client_sock, "% ", 2);
		recv(client_sock,input,sizeof(input),0);
		//cin.getline(input, MAXC);
		input_str = input;
		if (input_str.empty()){
			continue;
		}
		if (input_str.find("exit") != string::npos){
			close(client_sock);
			exit(0);
			break;
		}
		char *temp_str1 = input;
		char *temp_str = strtok(temp_str1,"\r\n");
		char *arglist[nu + 1];
		int index = 0;
		char *token;
		token = strtok(temp_str," ");
		arglist[index] = token;
		index++;
		while(token){
			token = strtok(NULL," ");
			arglist[index] = token;
			index++;
		}
		arglist[index-1] = 0;
		int temp_pn = index;
		char *command[nu][MAXC+1] = {0};
		//char *command[nu][MAXC + 1] = {0};
		int command_index = 0;
		int counter = 0;
		index = 0;
		token = arglist[index];
		if (string(arglist[0]) == "printenv" ){
			char* path;
			path = getenv(arglist[1]);
			if (path != NULL){
				cout << path << endl;
			}
			continue;	
		}
		if (string(arglist[0]) == "setenv"){
			setenv(arglist[1], arglist[2], 1);
			continue;
		}
		while(token){
			if (string(token).find("|") != string::npos || string(token).find(">") != string::npos || string(token).find("!") != string::npos){
				command[counter][command_index] = 0;
				index++;
				token = arglist[index];
				command_index = 0;
				counter++;
			}
			else{
				command[counter][command_index] = token;
				index++;
				command_index++;
				token = arglist[index];
			}
		}
		string pipe_with_number(arglist[temp_pn-2]);
		int pipe_number = count_pipe(input_str);
		int minus_pipe = 0;
		int numpiped_num = 0;
		int flag = -1;
		if (pipe_with_number.find("|") != string::npos){
			char *ab = strtok(arglist[temp_pn-2], "|");
			numpiped_num = strtol(ab, NULL, 10);
			for (int i = 0; i < glo.size(); i++){
				if (numpiped_num == glo[i]){
					flag = i;
					break;
				}
			}
			glo.push_back(numpiped_num);
			if (flag != -1){
				which_pipe.push_back(which_pipe[flag]);
				minus_pipe++;
			}
			else{
				which_pipe.push_back(pipe_counter + pipe_number - 1);
			}
		}
		if (pipe_with_number.find("!") != string::npos){
			char *ab = strtok(arglist[temp_pn-2], "!");
			numpiped_num = strtol(ab, NULL, 10);
			for (int i = 0; i < glo.size(); i++){
				if (numpiped_num == glo[i]){
					flag = i;
					break;
				}
			}
			glo.push_back(numpiped_num);
			if (flag != -1){
				which_pipe.push_back(which_pipe[flag]);
				minus_pipe++;
			}
			else{
				which_pipe.push_back(pipe_counter + pipe_number - 1);
			}
		}
		int where_implement = -1;
		for (int i = 0; i < glo.size(); i++){
			if (glo[i] == 0){
				where_implement = i; //第幾格存放要執行的pipe
				break;
			}
		}

		int symbol = -1; 
		symbol = classfy(input_str);
		if (numpiped_num > 0){
			symbol = 3;
		}
		if (where_implement != -1){
			symbol = 4;
		}
		
		if (symbol == 0){
			simple_com(&command, where_implement);
		}
		if (symbol == 1){
			redirection(&command, where_implement);
		}

		if (symbol == 2){
			//int pipe_number = count_pipe(input_str);
			mypipe(&command,pipe_counter,pipe_number, where_implement);
		}
		if (symbol == 3){
			numberedpipe(&command, pipe_counter, pipe_number, flag, where_implement);
		}
		if (symbol == 4){
			int number_symbol = -1;
			number_symbol = classfy(input_str);
			if (numpiped_num > 0){
				number_symbol = 3;
			}
			if (number_symbol == 0){
				simple_com(&command, where_implement);
			}
			if (number_symbol == 1){
				redirection(&command, where_implement);
			}
			if (number_symbol == 2){
				mypipe(&command,pipe_counter,pipe_number, where_implement);
			}
			if (number_symbol == 3){
				numberedpipe(&command, pipe_counter, pipe_number, flag, where_implement);
			}
		}
		

		pipe_counter = pipe_counter + pipe_number;
		for(int i = 0; i < glo.size(); i++){
			glo[i]--;
		}
		
	}
}


int main(int argc, char *argv[]){
	int sockfd = 0, index = 0, sockpid, port;
	port = atoi(argv[1]);
	sockfd = passivesock(port);
	cout << "waiting for connection..." << endl;
	while(1){
		int temp = index;	
		client_sock[temp] = accept(sockfd,(struct sockaddr*) &clientInfo, (socklen_t*)&addrlen);
		index++;
		if (client_sock[temp] == -1){
			cerr << "connect error!!" << endl;
			exit(1);
		}
		
		sockpid = fork();
		if (sockpid < 0){
			cerr << "fork error!!" << endl;
			exit(1);
		}
		else if (sockpid == 0){
			dup2(client_sock[temp], STDOUT_FILENO);
			dup2(client_sock[temp], STDERR_FILENO);
			close(sockfd);
			np(client_sock[temp]);
			close(client_sock[temp]);
		}
		else{
			close(client_sock[temp]);
			waitpid(sockpid, NULL, WNOHANG);
		}
	}	
}
