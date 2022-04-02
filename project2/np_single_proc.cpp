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
#include <arpa/inet.h>
#include <errno.h>
#include <algorithm>
using namespace std;

#define MAXC 2700
#define nu 1000
//int pipe1[nu][2] = {0};
string input_str;
//vector<int> glo, which_pipe;
//int pipe_counter = 0;
int addrlen = 0;
//int client_sock[nu] = {0};
struct sockaddr_in serverInfo,clientInfo;
bool id_sta[30+1] = {false}; //false代表沒人用
struct user{
	int id;
	int fd;
	int pipe_counter = 0;
	int pipe1[nu][2] = {0};
	string path = "bin:.";
	vector<int> glo, which_pipe, to_whom, which_user_pipe;
	string nickname;
	string ip_port;
};
struct less_than_key {
    inline bool operator() (const user& struct1, const user& struct2){
        return (struct1.id < struct2.id);
    }
};

vector<user> alluser;

int passivesock(int port){
	int sockfd = 0;
	sockfd = socket(AF_INET , SOCK_STREAM , 0);

	if (sockfd == -1){
		cerr << "Fail to create a socket." << endl;;
	}
	//struct sockaddr_in serverInfo,clientInfo;
	//socklen_t* addrlen = sizeof(clientInfo);
	addrlen = sizeof(clientInfo);
	bzero(&serverInfo,sizeof(serverInfo));


	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = htons(port);
	bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	listen(sockfd,5);
	return sockfd;
		
}
int* store_std(){
    int* arr= new int[3];
    arr[0] = dup(STDIN_FILENO);
    arr[1] = dup(STDOUT_FILENO);
    arr[2] = dup(STDERR_FILENO);
    return arr;
}
void restore_std(int* arr){
    dup2(arr[0], STDIN_FILENO);
    dup2(arr[1], STDOUT_FILENO);
    dup2(arr[2], STDERR_FILENO);
    close(arr[0]);
    close(arr[1]);
    close(arr[2]);
    delete[] arr;
}

bool replace(string& str, const string& from, const string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


int get_user_fd(int fd){
	int which_user;
	 for (int i = 0; i < alluser.size(); i++){
                if (alluser[i].fd == fd){
                        which_user = i;
                        break;
                }
        }
	return which_user;
}

int get_user_id(int id){
	int which_user = -1;
	 for (int i = 0; i < alluser.size(); i++){
                if (alluser[i].id == id){
                        which_user = i;
                        break;
                }
        }
	return which_user;
}

void redirection(char* (*command)[nu][MAXC+1], int where_implement, int which_user){
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
			dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
			int fd1 = open(*(*command)[1], O_CREAT|O_WRONLY, 0666);
			close(1);
			int fd2 = dup(fd1);
			int er = execvp(*(*command)[0],(*command)[0]);
			if (er == -1){
				cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
				exit(1);
			}
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
			close(fd1);
			close(fd2);
			exit(0);		
		}

	}
	else{
		if (where_implement != -1){
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);			
		}
		int wait_rv = wait(NULL);
		//write(client_sock,"% ", 2);
	}
}
void sig_handler(int sig){
/*
        int retval;
        retval = wait(NULL);*/
	wait(0);
}
void simple_com(char* (*command)[nu][MAXC+1], int where_implement, int which_user, string input){
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
			if (input.find(">") != string::npos){
				close(0);
				dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
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
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
			}
			else{

			
				close(0);
				dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
				int er = execvp(*(*command)[0],(*command)[0]);
				if (er == -1){
					cerr << "Unknown command: " << "[" << *(*command)[0] << "]" << "." << endl;
					exit(1);
				}
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
				exit(0);
			}		
		}
	}
	else{
		if (where_implement != -1){
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
			close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);			
		}

		int wait_rv = wait(NULL);
	}
}


void mypipe(char* (*command)[nu][MAXC+1], int &pipe_counter, int pipe_number, int where_implement, int which_user){
	for(int i = pipe_counter; i < pipe_counter + pipe_number; i++){
		int ret = pipe(alluser[which_user].pipe1[i]);
		if (ret == -1){
			cerr << "pipe error!!" << endl;
			exit(1);
		}
	}
	for(int i = pipe_counter; i < pipe_counter + pipe_number + 1; i++){
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
					dup(alluser[which_user].pipe1[pipe_counter][1]);
					if (input_str.find("!") != string::npos){//指令直接接!
						close(2);
						dup(alluser[which_user].pipe1[pipe_counter][1]);
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == pipe_counter){
							close(alluser[which_user].pipe1[j][0]);
						}
						else{
							close(alluser[which_user].pipe1[j][0]);
							close(alluser[which_user].pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(alluser[which_user].pipe1[pipe_counter][1]);
				}
				else{
					close(0);
					dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
					close(1);
					dup(alluser[which_user].pipe1[pipe_counter][1]);
					if (input_str.find("!") != string::npos){//指令直接接!
						close(2);
						dup(alluser[which_user].pipe1[pipe_counter][1]);
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == pipe_counter){
							close(alluser[which_user].pipe1[j][0]);
						}
						else{
							close(alluser[which_user].pipe1[j][0]);
							close(alluser[which_user].pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(alluser[which_user].pipe1[pipe_counter][1]);
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);

				}

			}
			else if (i == pipe_counter + pipe_number){ //最後一個child process
				close(0);
				dup(alluser[which_user].pipe1[i-1][0]);
				//處理全部的pipe
				for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
					if (j == pipe_counter + pipe_number - 1){//最後一個pipe
						close(alluser[which_user].pipe1[j][1]);
					}
					else{
						close(alluser[which_user].pipe1[j][0]);
						close(alluser[which_user].pipe1[j][1]);
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
				close(alluser[which_user].pipe1[i-1][0]);
			}
			else{
				close(0);
				dup(alluser[which_user].pipe1[i-1][0]);
				close(1);
				dup(alluser[which_user].pipe1[i][1]);
				if (i == pipe_counter + pipe_number - 1){
					if (input_str.find("!") != string::npos){
						close(2);
						dup(alluser[which_user].pipe1[i][1]);
					}
				}
				for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
					if (j == i-1){//process 的前一個pipe
						close(alluser[which_user].pipe1[j][1]);
					}
					else if (j == i){//process後的pipe
						close(alluser[which_user].pipe1[j][0]);
					}
					else{//其他pipe
						close(alluser[which_user].pipe1[j][0]);
						close(alluser[which_user].pipe1[j][1]);
					}
				}
				int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
                                if (er == -1){
                                        cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
                                        exit(1);
                                }
				close(alluser[which_user].pipe1[i-1][0]);
				close(alluser[which_user].pipe1[i][1]);
			}	
		}
		else{
	
			if (where_implement != -1){
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
				close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);			
			}
			if (i == pipe_counter + pipe_number){
				for (int i = pipe_counter; i < pipe_counter + pipe_number; i++){
					close(alluser[which_user].pipe1[i][0]);
					close(alluser[which_user].pipe1[i][1]);
				}
				int wait_rv = wait(NULL);
			}
			
			else{
				int wait_rv = waitpid(newpid, NULL, WNOHANG);
				//signal(SIGCHLD,sig_handler);
			}	

		}
	}
}

void numberedpipe(char* (*command)[nu][MAXC+1], int &pipe_counter, int pipe_number, int flag, int where_implement, int which_user){
	if (flag == -1){
		for(int i = pipe_counter; i < pipe_counter + pipe_number; i++){
			int ret = pipe(alluser[which_user].pipe1[i]);
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
						dup(alluser[which_user].pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[pipe_counter][1]);
					}
					else{
						close(0);
						dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
						close(1);
						dup(alluser[which_user].pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[pipe_counter][1]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);

					}
				}
				else{
					close(0);
					dup(alluser[which_user].pipe1[i-1][0]);
					close(1);
					dup(alluser[which_user].pipe1[i][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(alluser[which_user].pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == i-1){//process 的前一個pipe
							close(alluser[which_user].pipe1[j][1]);
						}
						else if (j == i){//process後的pipe
							close(alluser[which_user].pipe1[j][0]);
						}
						else{//其他pipe
							close(alluser[which_user].pipe1[j][0]);
							close(alluser[which_user].pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(alluser[which_user].pipe1[i-1][0]);
					close(alluser[which_user].pipe1[i][1]);
				}	
			}
			else{

				if (where_implement != -1){
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);			
				}
					
				if (i == pipe_counter + pipe_number - 1){
					for (int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
						close(alluser[which_user].pipe1[i][0]);
						close(alluser[which_user].pipe1[i][1]);
					}
					int wait_rv = wait(NULL); //
				}

				int wait_rv = waitpid(newpid,NULL,WNOHANG);
				signal(SIGCHLD,sig_handler);

			}
		}//for
	}	
	else{
		for(int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
			int ret = pipe(alluser[which_user].pipe1[i]);
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
						dup(alluser[which_user].pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[pipe_counter][1]);
					}
					else{
						close(0);
						dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
						close(1);
						dup(alluser[which_user].pipe1[pipe_counter][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[pipe_counter][1]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);

					}	

				}
				else if (i == pipe_counter && pipe_number == 1){ //第一個child process
					if (where_implement == -1){
						close(1);
						dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
					}
					else{
						close(0);
						dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);
						close(1);
						dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][0]);

						if (input_str.find("!") != string::npos){//指令直接接!
							close(2);
							dup(alluser[which_user].pipe1[pipe_counter][1]);
						}
						for (int j = pipe_counter; j < pipe_counter + pipe_number - 1; j++){
							if (j == pipe_counter){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
						close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);


					}	
				}
				else if (i == pipe_counter + pipe_number - 1 && pipe_number > 1){
					close(0);
					dup(alluser[which_user].pipe1[i-1][0]);
					close(1);
					dup(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(alluser[which_user].pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number - 1; j++){
						if (j == i-1){//process 的前一個pipe
							close(alluser[which_user].pipe1[j][1]);
						}
						else{//其他pipe
							close(alluser[which_user].pipe1[j][0]);
							close(alluser[which_user].pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(alluser[which_user].pipe1[i-1][0]);
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[flag]][1]);
					
				}
				else{
					close(0);
					dup(alluser[which_user].pipe1[i-1][0]);
					close(1);
					dup(alluser[which_user].pipe1[i][1]);
					if (i == pipe_counter + pipe_number - 1){
						if (input_str.find("!") != string::npos){
							close(2);
							dup(alluser[which_user].pipe1[i][1]);
						}
					}
					for (int j = pipe_counter; j < pipe_counter + pipe_number; j++){
						if (j == i-1){//process 的前一個pipe
							close(alluser[which_user].pipe1[j][1]);
						}
						else if (j == i){//process後的pipe
							close(alluser[which_user].pipe1[j][0]);
						}
						else{//其他pipe
							close(alluser[which_user].pipe1[j][0]);
							close(alluser[which_user].pipe1[j][1]);
						}
					}
					int er = execvp(*(*command)[i-pipe_counter],(*command)[i-pipe_counter]);
					if (er == -1){
						cerr << "Unknown command: " << "[" << *(*command)[i-pipe_counter] << "]" << "." << endl;
						exit(1);
					}
					close(alluser[which_user].pipe1[i-1][0]);
					close(alluser[which_user].pipe1[i][1]);
				}	
			}
			else{
	
				if (where_implement != -1){
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][0]);
					close(alluser[which_user].pipe1[alluser[which_user].which_pipe[where_implement]][1]);			
				}

				if (i == pipe_counter + pipe_number - 1){
					for (int i = pipe_counter; i < pipe_counter + pipe_number - 1; i++){
						close(alluser[which_user].pipe1[i][0]);
						close(alluser[which_user].pipe1[i][1]);
					}
					int wait_rv = wait(NULL);
				}

				int wait_rv = waitpid(newpid,NULL,WNOHANG);
				//if (i == pipe_counter + pipe_number - 1){
				//	int wait_rv = wait(NULL);
				//}
				//else{
				//	int wait_rv = waitpid(newpid,NULL,WNOHANG);
				//}

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
void np(int client_sock, string input){
	int which_user = get_user_fd(client_sock);
	
	int *std;
	std = store_std();
	dup2(client_sock, STDOUT_FILENO);
	dup2(client_sock, STDERR_FILENO);
	string com = input;
	if (com.find("\n") != string::npos){
		replace(com, "\n", "");
	}
	if (com.find("\r") != string::npos){
                replace(com, "\r", "");
        }
	//char *temp_str1 = (char *)input.c_str();
	//char *temp_str = strtok(temp_str1,"\r\n");
	char *temp_str = (char *)com.c_str();
	char *arglist[MAXC + 1];
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
	char *command[nu][MAXC + 1] = {0};
	int command_index = 0;
	int counter = 0;
	index = 0;
	token = arglist[index];
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
		for (int i = 0; i < alluser[which_user].glo.size(); i++){
			if (numpiped_num == alluser[which_user].glo[i]){
				flag = i;
				break;
			}
		}
		alluser[which_user].glo.push_back(numpiped_num);
		if (flag != -1){
			alluser[which_user].which_pipe.push_back(alluser[which_user].which_pipe[flag]);
			//minus_pipe++;
		}
		else{
			alluser[which_user].which_pipe.push_back(alluser[which_user].pipe_counter + pipe_number - 1);
		}
	}
	if (pipe_with_number.find("!") != string::npos){
		char *ab = strtok(arglist[temp_pn-2], "!");
		numpiped_num = strtol(ab, NULL, 10);
		for (int i = 0; i < alluser[which_user].glo.size(); i++){
			if (numpiped_num == alluser[which_user].glo[i]){
				flag = i;
				break;
			}
		}
		alluser[which_user].glo.push_back(numpiped_num);
		if (flag != -1){
			alluser[which_user].which_pipe.push_back(alluser[which_user].which_pipe[flag]);
			//minus_pipe++;
		}
		else{
			alluser[which_user].which_pipe.push_back(alluser[which_user].pipe_counter + pipe_number - 1);
		}
	}
	int where_implement = -1;
	for (int i = 0; i < alluser[which_user].glo.size(); i++){
		if (alluser[which_user].glo[i] == 0){
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
		simple_com(&command, where_implement, which_user, input);
	}
	if (symbol == 1){
		redirection(&command, where_implement, which_user);
	}

	if (symbol == 2){
		mypipe(&command,alluser[which_user].pipe_counter,pipe_number, where_implement, which_user);
	}
	if (symbol == 3){
		numberedpipe(&command, alluser[which_user].pipe_counter, pipe_number, flag, where_implement, which_user);
	}

	if (symbol == 4){
		int number_symbol = -1;
		number_symbol = classfy(input_str);
		if (numpiped_num > 0){
			number_symbol = 3;
		}
		if (number_symbol == 0){
			simple_com(&command, where_implement, which_user, input);
		}
		if (number_symbol == 1){
			redirection(&command, where_implement, which_user);
		}
		if (number_symbol == 2){
			mypipe(&command,alluser[which_user].pipe_counter,pipe_number, where_implement, which_user);
		}
		if (number_symbol == 3){
			numberedpipe(&command, alluser[which_user].pipe_counter, pipe_number, flag, where_implement, which_user);
		}
	}
	

	alluser[which_user].pipe_counter = alluser[which_user].pipe_counter + pipe_number;
	/*for(int i = 0; i < alluser[which_user].glo.size(); i++){
		alluser[which_user].glo[i]--;
	}*/
	restore_std(std);
	write(client_sock, "% ", 2);
}
void broadcast(string message){
	for(int i = 0; i < alluser.size(); i++){
		write(alluser[i].fd,message.c_str(), message.length());
		//write(alluser[i].fd, "% ", 2);
	}
}
void welcome(int ssock){
	string ini_msg = "****************************************\n";
	string ini_msg2 = "** Welcome to the information server. **\n";
	string ini = ini_msg + ini_msg2 + ini_msg;
	write(ssock, ini.c_str(), ini.length());
	//write(ssock, ini_msg2, sizeof(ini_msg2));
	//write(ssock, ini_msg, sizeof(ini_msg));	
	//string message = "*** User '(no name)' entered from " + ip_port + ". ***\r\n";
	//broadcast(message);
	//write(ssock, "% ", 2);
}
void enter(int ssock, string ip_port){
	string message = "*** User '(no name)' entered from " + ip_port + ". ***\n";
        broadcast(message);
	//write(ssock, "% ", 2);
}
void add_client(int id, int ssock, string ip_port){
	user client;
	client.id = id;
	client.fd = ssock;
	client.ip_port = ip_port;
	client.nickname = "(no name)";
	client.pipe_counter = 0;
	client.pipe1[nu][2] = {0};
	alluser.push_back(client);
}


void name(int fd, string yourname){
	int which_user = get_user_fd(fd);
	int flag =  -1;
	for (int i = 0; i < alluser.size(); i++){
		if (alluser[i].nickname.compare(yourname) == 0){
			flag = 0;//有相同名字已經存在
			break;	
		}
	}
	if (flag == -1){
		alluser[which_user].nickname = yourname;
		string message = "*** User from " + alluser[which_user].ip_port + " is named '"+ yourname + "'. ***\n";
		//write(fd, message.c_str(), message.length());
		//write(fd, "% ", 2);
		broadcast(message);
		write(fd, "% ", 2);
	}
	else{
		string message = "*** User '" + yourname + "' already exists. ***\r\n";
		write(fd, message.c_str(), message.length());
		write(fd, "% ", 2);
	}
}
void who(int fd){
	sort(alluser.begin(), alluser.end(), less_than_key());
	string message = "<ID>\t<nickname>\t<IP:port>\t<indicate me>\r\n";
	for (int i = 0; i < alluser.size(); i++){
		if (alluser[i].fd == fd){
			message += to_string(alluser[i].id) + "\t" + alluser[i].nickname + "\t" + alluser[i].ip_port + "\t" + "<-me\r\n";
		}
		else{
			message += to_string(alluser[i].id) + "\t" + alluser[i].nickname + "\t" + alluser[i].ip_port + "\n";
		}
	}
	write(fd, message.c_str(), message.length());	
	write(fd, "% ", 2);
}
int assign_id(){
	int temp_id;
	for(int i = 1; i <= 30; i++){
		if (id_sta[i] == false){
			temp_id = i;
			id_sta[i] = true;
			break;
		}
	}
	return temp_id;	
}

void tell(int fd, int dest_id, string send_message){
	int which_user;
	int dest_user;
	if (id_sta[dest_id] == true){
		which_user = get_user_fd(fd);
		for (int i = 0; i < alluser.size(); i++){
			if (alluser[i].id == dest_id){
				dest_user = i;
				break;
			}
		}
		string message = "*** "+ alluser[which_user].nickname +" told you ***: " + send_message + "\n";
		write(alluser[dest_user].fd, message.c_str(), message.length());
		//write(alluser[dest_user].fd, "% ", 2);
	}
	else{
		string err_message = "*** Error: user #"+ to_string(dest_id) +" does not exist yet. ***" + "\n";
		write(fd, err_message.c_str(), err_message.length());
	}
	write(fd, "% ", 2);
}

void yell(int fd, string send_message){
	int which_user = get_user_fd(fd);
	string message	= "*** "+ alluser[which_user].nickname + " yelled ***: " + send_message + "\n";
	broadcast(message);
	write(fd, "% ", 2);
}
void out(int fd){
	int which_user = get_user_fd(fd);
    string message  = "*** User '"+ alluser[which_user].nickname + "' left. ***" + "\n";
	id_sta[alluser[which_user].id] = false;
	for (int i = 0; i < alluser.size(); i++){
		if (i != which_user){
			for (int j = 0; j < alluser[i].to_whom.size(); j++){
				if (alluser[i].to_whom[j] == alluser[which_user].id){
					alluser[i].to_whom.erase(alluser[i].to_whom.begin() + j);
					alluser[i].which_user_pipe.erase(alluser[i].which_user_pipe.begin() + j);
					break;
				}	
			}
		}

	}
	alluser[which_user].to_whom.clear();
	alluser[which_user].which_user_pipe.clear();
	alluser.erase(alluser.begin() + which_user);
        broadcast(message);
	close(fd);
}
void userpipe(int fd, string input){
	int which_user = get_user_fd(fd);
	string com = input;
	if (com.find("\n") != string::npos){
		replace(com, "\n", "");
	}
	if (com.find("\r") != string::npos){
                replace(com, "\r", "");
        }
	char *temp_str = (char *)com.c_str();
	//char *temp_str1 = (char *)input.c_str();
	//char *temp_str = strtok(temp_str1,"\r\n");
	string s = temp_str;
	char *arglist[MAXC + 1];
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
	char *command[nu][MAXC + 1] = {0};
	int command_index = 0;
	int counter = 0;
	index = 0;
	token = arglist[index];

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
	int pipe_number = count_pipe(input);
	int minus_pipe = 0;
	int numpiped_num = 0;
	int flag = -1;
	char *ab = strtok(arglist[temp_pn-2], ">");
	numpiped_num = strtol(ab, NULL, 10);//擷取數字
	int dest_user = get_user_id(numpiped_num);
	for (int i = 0; i < alluser[which_user].to_whom.size(); i++){
		if (numpiped_num == alluser[which_user].to_whom[i]){//給那個人的pipe已經存在了
			flag = i;
			break;
		}
	}//flag != -1 代表裡面已經有相同的pipe了
	if (dest_user != -1){//代表目的地存在
		if (flag == -1){
			for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number + 1; i++){
				int ret = pipe(alluser[which_user].pipe1[i]);
				if (ret == -1){
					cerr << "pipe error!!" << endl;
					exit(1);
				}
			}
			for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number + 1; i++){
				int newpid;
				signal(SIGCHLD,sig_handler);
				newpid = fork();
				if (newpid == -1){
					cerr << "error" << endl;
				}
				else if (newpid == 0){
					if (i == alluser[which_user].pipe_counter){//第一個chile
						close(1);
						dup(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
						for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number + 1; j++){
							if (j == i){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*command[0],command[0]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
					}
					else{//最後一個child
						close(0);
						dup(alluser[which_user].pipe1[i - 1][0]);
						close(1);
						dup(alluser[which_user].pipe1[i][1]);
						for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number + 1; j++){
							if (j == i - 1){
								close(alluser[which_user].pipe1[j][1]);
							}
							else if (j == i){
								close(alluser[which_user].pipe1[j][0]);
							}
							else{
								close(alluser[which_user].pipe1[j][0]);
								close(alluser[which_user].pipe1[j][1]);
							}
						}
						int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
						if (er == -1){
							cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
							exit(1);
						}
						close(alluser[which_user].pipe1[i - 1][0]);
						close(alluser[which_user].pipe1[i][1]);
					}//else if (i == alluser[which_user].pipe_counter + pipe_number + 1)
					
				}//else if(newpid==0)
				else{//parent
					if (i == alluser[which_user].pipe_counter + pipe_number){
						for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number; i++){
							close(alluser[which_user].pipe1[i][0]);
							close(alluser[which_user].pipe1[i][1]);
						}
						//int wait_rv = waitpid(newpid, NULL, WNOHANG);
						int wait_rv = wait(NULL);
					}
					
					else{
						//int wait_rv = wait(NULL);
						int wait_rv = waitpid(newpid, NULL, WNOHANG);
						//signal(SIGCHLD,sig_handler);
					}	
				}//parent else
			}
			alluser[which_user].pipe_counter = alluser[which_user].pipe_counter + pipe_number;
			alluser[which_user].to_whom.push_back(numpiped_num);
			alluser[which_user].which_user_pipe.push_back(alluser[which_user].pipe_counter);
			string message = "*** "+ alluser[which_user].nickname +" (#"+ to_string(alluser[which_user].id)+") just piped '"+ s + "' to "+ alluser[dest_user].nickname + " (#" + to_string(numpiped_num) + ") ***\n";
			broadcast(message);
			//write(fd, message.c_str(), message.length());
			alluser[which_user].pipe_counter++; //因為有> 所以多一個pipe
		}
		else{
			string message = "*** Error: the pipe #" + to_string(alluser[which_user].id) + "->#"+ to_string(numpiped_num)+" already exists. ***\n";
			write(fd, message.c_str(), message.length());

		}
	}
	else{
		string message = "*** Error: user #" + to_string(numpiped_num)+ " does not exist yet. ***\n";
		write(fd, message.c_str(), message.length());
	}
	//alluser[which_user].pipe_counter++; //因為有> 所以多一個pipe
	write(fd, "% ", 2);
}

void get_userpipe(int fd, string input){
	int which_user = get_user_fd(fd); //使用者
	string com = input;
	if (com.find("\n") != string::npos){
		replace(com, "\n", "");
	}
	if (com.find("\r") != string::npos){
                replace(com, "\r", "");
    }
    char *temp_str = (char *)com.c_str();
	string str = temp_str;
	char *arglist[MAXC + 1];
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
	char *command[nu][MAXC + 1] = {0};
	int command_index = 0;
	int counter = 0;
	index = 0;
	token = arglist[index];
	char *command_no_one[nu][MAXC + 1] = {0};
	int where_small, where_big = -1; //小於在字串哪個位置
	for (int i = 0; i < temp_pn - 1; i++){
		if (string(arglist[i]).find("<") != string::npos){
			where_small = i;
			break;
		}
	}
	for (int i = 0; i < temp_pn - 1; i++){
		if (string(arglist[i]).find(">") != string::npos){
			where_big = i;
			break;
		}
	}

	int small_flag = 0;
	while(token){
		if (string(token).find("|") != string::npos || string(token).find(">") != string::npos || string(token).find("!") != string::npos || string(token).find("<") != string::npos){
			if (string(token).find("<") != string::npos){
				small_flag = 1;
				index++;
				token = arglist[index];
				continue;
			}
			command[counter][command_index] = 0;
			index++;
			token = arglist[index];
			command_index = 0;
			counter++;
		}
		else{
			command[counter][command_index] = token;
			if (small_flag == 1){
				command_no_one[counter-1][command_index] = token;
			}
			index++;
			command_index++;
			token = arglist[index];
		}
	}
	string pipe_with_number = "";
	string big_number = "";
	if (where_small != -1){
		pipe_with_number = string(arglist[where_small]);
		//string pipe_with_number(arglist[where_small]);
	}
	if (where_big != -1){
		big_number = string(arglist[where_big]);
		//string big_number(arglist[where_big]);
	}
	string user_number_pipe(arglist[temp_pn-2]);
	int numpiped_num, numpiped_num_dest, after_pipe_number = 0;
	int flag = -1; //看有誰要傳過來的
	int *std;
	if (pipe_with_number.find("<") != string::npos){ //抓小於後面的數字
		char *ab = strtok(arglist[where_small], "<");
		numpiped_num = strtol(ab, NULL, 10);
	}
	if (big_number.find(">") != string::npos){ //抓大於後面的數字
		char *ab = strtok(arglist[where_big], ">");
		numpiped_num_dest = strtol(ab, NULL, 10);
	}
	if (user_number_pipe.find("|") != string::npos){ //抓pipe後面的數字
		char *ab = strtok(arglist[temp_pn-2], "|");
		after_pipe_number = strtol(ab, NULL, 10);
	}

	int dest_user_fd = get_user_id(numpiped_num);//從哪裡接過來的fd <1 從1過來
	int send_dest_fd = get_user_id(numpiped_num_dest); //送給誰
	int pipe_number = 0;
	pipe_number = count_pipe(input);
	if (dest_user_fd != -1){//代表有這個ID
		for (int i = 0; i < alluser[dest_user_fd].to_whom.size(); i++){
			if (alluser[dest_user_fd].to_whom[i] == alluser[which_user].id){
				flag = i; //代表有東西要傳過來
			}
		}
		if (flag != -1){
			string message = "*** "+ alluser[which_user].nickname +" (#"+ to_string(alluser[which_user].id)+") just received from " + alluser[dest_user_fd].nickname + " (#" + to_string(numpiped_num) + ") by '" + str + "' ***\n";
			broadcast(message);

			std = store_std();
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
				
			if (pipe_number > 0){//代表接收進來的資料後面還有pipe
				for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number ; i++){
					int ret = pipe(alluser[which_user].pipe1[i]);
					if (ret == -1){
						cerr << "pipe error!!" << endl;
						exit(1);
					}
				}
			}
			if (big_number.find(">") != string::npos){//建立最後還要傳給別人的pipe
				
				int ret = pipe(alluser[which_user].pipe1[alluser[which_user].pipe_counter + pipe_number]);
				if (ret == -1){
					cerr << "pipe error!!" << endl;
					exit(1);
				}
				string message = "*** "+ alluser[which_user].nickname +" (#"+ to_string(alluser[which_user].id)+") just piped '"+ str + "' to "+ alluser[send_dest_fd].nickname + " (#" + to_string(numpiped_num_dest) + ") ***\n";
				broadcast(message);
				
			}
				
			for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number + 1; i++){
				if (i == alluser[which_user].pipe_counter + pipe_number && user_number_pipe.find("|") != string::npos){ //如果是numbered pipe 則最後一個不fork
					continue;
				}
				int newpid;
				signal(SIGCHLD,sig_handler);
				newpid = fork();
				if (newpid == -1){
					cerr << "error" << endl;
				}
				else if (newpid == 0){
					if (pipe_number > 0){//接收後面還有pipe
						if (big_number.find(">") != string::npos){
							if (i == alluser[which_user].pipe_counter){//第一個子程序
								close(0);
								dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
								close(1);
								dup(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][0]);
								
								int er = execvp(*command[0],command[0]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);	
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
							}
							else{
								close(0);
								dup(alluser[which_user].pipe1[i - 1][0]);
								close(1);
								dup(alluser[which_user].pipe1[i][1]);
								for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number + 1; j++){
									if (j == i - 1){
										close(alluser[which_user].pipe1[j][1]);
									}
									else if (j == i){
										close(alluser[which_user].pipe1[j][0]);
									}
									else{
										close(alluser[which_user].pipe1[j][0]);
										close(alluser[which_user].pipe1[j][1]);
									}
								}
								int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[which_user].pipe1[i - 1][0]);
								close(alluser[which_user].pipe1[i][1]);
							}
						}
						else if (user_number_pipe.find("|") != string::npos){
							if (i == alluser[which_user].pipe_counter){//第一個子程序
								close(0);
								dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
								close(1);
								dup(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][0]);
								
								int er = execvp(*command[0],command[0]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);	
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
							}
							else{
								close(0);
								dup(alluser[which_user].pipe1[i - 1][0]);
								close(1);
								dup(alluser[which_user].pipe1[i][1]);
								for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number; j++){
									if (j == i - 1){
										close(alluser[which_user].pipe1[j][1]);
									}
									else if (j == i){
										close(alluser[which_user].pipe1[j][0]);
									}
									else{
										close(alluser[which_user].pipe1[j][0]);
										close(alluser[which_user].pipe1[j][1]);
									}
								}
								int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[which_user].pipe1[i - 1][0]);
								close(alluser[which_user].pipe1[i][1]);
							}
						}
						else{//最後沒有要傳給別人
							if (i == alluser[which_user].pipe_counter){//第一個子程序
								close(0);
								dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
								close(1);
								dup(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][0]);
								
								int er = execvp(*command[0],command[0]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);	
								close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
							}
							else if (i == alluser[which_user].pipe_counter + pipe_number){
								close(0);
								dup(alluser[which_user].pipe1[i - 1][0]);
								for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number; j++){
									if (j == i - 1){
										close(alluser[which_user].pipe1[j][1]);
									}
									else{
										close(alluser[which_user].pipe1[j][0]);
										close(alluser[which_user].pipe1[j][1]);
									}
								}
								if (input_str.find(">") != string::npos){
									int fd1 = open(*command[i-alluser[which_user].pipe_counter+1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
									close(1);
									int fd2 = dup(fd1);
									int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
									if (er == -1){
										cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
										exit(1);
									}
									close(fd1);
									close(fd2);
								}
								else{
									int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
									if (er == -1){
										cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
										exit(1);
									}
								}

								close(alluser[which_user].pipe1[i - 1][0]);
							}
							else{
								close(0);
								dup(alluser[which_user].pipe1[i - 1][0]);
								close(1);
								dup(alluser[which_user].pipe1[i][1]);
								for (int j = alluser[which_user].pipe_counter; j < alluser[which_user].pipe_counter + pipe_number; j++){
									if (j == i - 1){
										close(alluser[which_user].pipe1[j][1]);
									}
									else if (j == i){
										close(alluser[which_user].pipe1[j][0]);
									}
									else{
										close(alluser[which_user].pipe1[j][0]);
										close(alluser[which_user].pipe1[j][1]);
									}
								}
								int er = execvp(*command[i-alluser[which_user].pipe_counter],command[i-alluser[which_user].pipe_counter]);
								if (er == -1){
									cerr << "Unknown command: " << "[" << *command[i-alluser[which_user].pipe_counter] << "]" << "." << endl;
									exit(1);
								}
								close(alluser[which_user].pipe1[i - 1][0]);
								close(alluser[which_user].pipe1[i][1]);
							}
						}//else 中間子程序
						
					}//if (pipe_number >0 )
					else{
						if (big_number.find(">") != string::npos){ //傳給別人
							close(0);
							dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
							close(1);
							dup(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);
							close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][0]);
							int er = execvp(*command[0],command[0]);
							if (er == -1){
								cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
								exit(1);
							}
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
							close(alluser[which_user].pipe1[alluser[which_user].pipe_counter][1]);

							string message = "*** "+ alluser[which_user].nickname +" (#"+ to_string(alluser[which_user].id)+") just piped '" + str + "' to "+ alluser[send_dest_fd].nickname + " (#" + to_string(numpiped_num_dest) + ") ***\n";
							broadcast(message);
						}
						else if (input.find(">") != string::npos){ //重定位
							close(0);
							dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
							int fd1 = open(*command[1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
							close(1);
							int fd2 = dup(fd1);
							int er = execvp(*command[0],command[0]);
							if (er == -1){
								cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
								exit(1);
							}
							close(fd1);
							close(fd2);
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);	
						}

						else{
							close(0);
							dup(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);
							
							int er = execvp(*command[0],command[0]);
							if (er == -1){
								cerr << "Unknown command: " << "[" << *command[0] << "]" << "." << endl;
								exit(1);
							}
							close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);
						}	
					}
				}
				else{
					if (i == alluser[which_user].pipe_counter){
						close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][0]);	
						close(alluser[dest_user_fd].pipe1[alluser[dest_user_fd].which_user_pipe[flag]][1]);	
					}

					if (pipe_number > 0){
						if (user_number_pipe.find("|") != string::npos){
							if (i == alluser[which_user].pipe_counter + pipe_number - 1){
								for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number - 1; i++){
									close(alluser[which_user].pipe1[i][0]);
									close(alluser[which_user].pipe1[i][1]);
								}
								int wait_rv = wait(NULL);
							}
							else{
								int wait_rv = waitpid(newpid, NULL, WNOHANG);	
							}
						}
						else{
							if (i == alluser[which_user].pipe_counter + pipe_number){
								for (int i = alluser[which_user].pipe_counter; i < alluser[which_user].pipe_counter + pipe_number; i++){
									close(alluser[which_user].pipe1[i][0]);
									close(alluser[which_user].pipe1[i][1]);
								}
								int wait_rv = wait(NULL);
							}
							else{
								int wait_rv = waitpid(newpid, NULL, WNOHANG);	
							}
						}
						
					}
					else{
						int wait_rv = wait(NULL);
					}
				}
			}
			if (big_number.find(">") != string::npos){
				alluser[which_user].to_whom.push_back(numpiped_num_dest);
				alluser[which_user].which_user_pipe.push_back(alluser[which_user].pipe_counter + pipe_number);
				alluser[which_user].pipe_counter++;
				
			}
			if (user_number_pipe.find("|") != string::npos){
				alluser[which_user].glo.push_back(after_pipe_number);
				alluser[which_user].which_pipe.push_back(alluser[which_user].pipe_counter + pipe_number - 1);
			}
			alluser[which_user].pipe_counter = alluser[which_user].pipe_counter + pipe_number;
			restore_std(std);
			alluser[dest_user_fd].to_whom.erase(alluser[dest_user_fd].to_whom.begin() + flag);//已經傳完後原來 to_whom的pipe要刪掉
			alluser[dest_user_fd].which_user_pipe.erase(alluser[dest_user_fd].which_user_pipe.begin() + flag);


		}//flag != -1
		else{//flag == -1 代表pipe還不存在
			string message = "*** Error: the pipe #"+ to_string(alluser[dest_user_fd].id)+"->#" + to_string(alluser[which_user].id) +" does not exist yet. ***\n";
			write(fd, message.c_str(), message.length());
		}
	}//dest_user_fd != -1
	else{//代表沒有這個ID
		string message = "*** Error: user #" + to_string(numpiped_num)+ " does not exist yet. ***\n";
		write(fd, message.c_str(), message.length());
	}
	//restore_std(std);
	write(fd, "% ", 2);
}



int main(int argc, char *argv[]){
	//setenv("PATH","bin:.",1);
	int msock,ssock;
	int port = atoi(argv[1]);
	fd_set rfds; //是否要向server讀資料的開關
	fd_set afds; //client 的包廂
	int fd, nfds;
	nfds = getdtablesize();//看rfds這個table的大小
	msock = passivesock(port);
	FD_ZERO(&afds);
	FD_SET(msock, &afds);
	int *std;
	cout << "waiting for connection..." << endl;
	while(1){
		//memcpy(&rfds, &afds, sizeof(rfds));
		rfds = afds;	
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,NULL) < 0){//check on/off (struct timeval *)0
			if (errno == 4) {
				select(nfds, &rfds, (fd_set *)0, (fd_set *)0,NULL);
			}
		}
		if (FD_ISSET(msock, &rfds)) {//接單 在開關包廂中看這個位置是否為是開
			//int ssock;
			ssock = accept(msock,(struct sockaddr*) &clientInfo, (socklen_t*)&addrlen);	
			if (ssock < 0){
				cerr << "acpt: " << strerror(errno) << endl;
			}
			char ip_arr[INET_ADDRSTRLEN];
			string ip = inet_ntop( AF_INET, &clientInfo.sin_addr, ip_arr, INET_ADDRSTRLEN);
			string port = to_string(ntohs(clientInfo.sin_port));
			string ip_port = ip + ":" + port;
			int id = assign_id(); 
			add_client(id, ssock, ip_port);
			welcome(ssock);
			enter(ssock, ip_port);
			FD_SET(ssock, &afds);
			write(ssock, "% ", 2);

		}
		for (fd=0; fd<nfds; fd++){//處理讀寫
			if (fd != msock && FD_ISSET(fd, &rfds)){
				int which_user = get_user_fd(fd);
				setenv("PATH",alluser[which_user].path.c_str(),1);
				char input[MAXC] = {0};
				recv(fd,input,sizeof(input),0);
				input_str = input;
				//char *temp_str1 = input;
				//char *temp_str = strtok(temp_str1,"\r\n");
				string com = input;
				if (com.find("\n") != string::npos){
					replace(com, "\n", "");
				}
				if (com.find("\r") != string::npos){
					replace(com, "\r", "");
				}
				char *temp_str = (char *)com.c_str();
				char *arglist[MAXC + 1] = {0};
				int index_sk = 0;
				char *token;
				token = strtok(temp_str," ");
				arglist[index_sk] = token;
				index_sk++;
				while(token){
					token = strtok(NULL," ");
					arglist[index_sk] = token;
					index_sk++;
				}
				arglist[index_sk-1] = 0;
				string pipe_with_number(arglist[index_sk-2]);
				if (com.find("printenv") != string::npos){
					char* ppath;
					ppath = getenv(arglist[1]);
					if (ppath != NULL){
						string message = string(ppath) + "\n";
						write(fd, message.c_str(), message.length());
					}
					write(fd, "% ", 2);
					continue;
				}
				if (com.find("setenv") != string::npos){
					setenv(arglist[1], arglist[2], 1);
					alluser[which_user].path = string(arglist[2]);
					write(fd, "% ", 2);
					continue;
				}
				if (com.empty()){
					continue;
				}
				if (com.find("exit") != string::npos){
					out(fd);
					FD_CLR(fd, &afds);
					continue;
				}
				else if (com.find("name") != string::npos){
					name(fd,arglist[1]);
				}
				else if (com.find("who") != string::npos){
					who(fd);
				}
				else if (com.find("tell") != string::npos){
					int dest = strtol(arglist[1], NULL, 10);
					string send_message = "";
					for(int i = 2; i < index_sk - 1; i++){
						send_message += arglist[i];
						if (i != index_sk - 2){
							send_message += " ";
						}
					}
					tell(fd, dest, send_message);
				}
				else if (com.find("yell") != string::npos){
					string send_message = "";
					for(int i = 1; i < index_sk - 1; i++){
                        send_message += arglist[i];
                        if (i != index_sk - 2){
                            send_message += " ";
                        }
                    }
					yell(fd, send_message);
				}

				else if (com.find("<") != string::npos){
					get_userpipe(fd, input);
				}
				else if (pipe_with_number.find(">") != string::npos){
					userpipe(fd, input);
				}
				else{
					np(fd, input);
				}
				for(int i = 0; i < alluser[which_user].glo.size(); i++){
					alluser[which_user].glo[i]--;
				}
			}
		}
		
	}// while	
}
