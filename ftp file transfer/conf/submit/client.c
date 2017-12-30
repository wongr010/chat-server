
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#define MAX_DATA 1000
#define MAX_LEN 100

struct lab3message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_LEN];
	unsigned char data[MAX_DATA];
};

char * type_dictionary[16] = {"LOGIN",
					  "LO_ACK",
					  "LO_NAK",
					  "EXIT",
					  "JOIN",
					  "JN_ACK",
					  "JN_NAK",
					  "LEAVE_SESS",
					  "NEW_SESS",
					  "NS_ACK",
					  "MESSAGE",
					  "QUERY",
					  "QU_ACK", 
					  "INVITE", 
					  "IN_ACK", 
					  "IN_NAK"};

int client_connect(const char *serverIP, const char *port, int * sockfd /*pas in sockfd by pointer reference*/){

			struct addrinfo hints, *res,*iter;
			int err;
			res = (struct addrinfo *)malloc(sizeof(struct addrinfo));
			memset(&hints, 0, sizeof(struct addrinfo));
			memset(res, 0, sizeof(struct addrinfo));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM; //use TCP
			hints.ai_protocol = IPPROTO_TCP;
			//hints.ai_flags=AI_PASSIVE;
			printf("Attempting to connect to server %s\n",serverIP);
			err = getaddrinfo(serverIP, port, &hints, &res);
			if(err<0){
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
				return 1;
			}
			
		
			//Open new socket
			
			int sockfd_local;
			for(iter=res; iter!=NULL;iter=res->ai_next){
				
				printf("nextt sock\n");
				sockfd_local = socket(iter->ai_family, iter->ai_socktype,
						iter->ai_protocol);
				if(sockfd_local == -1){
					printf("socket\n");
					continue;
				}
				
				else if(connect(sockfd_local,iter->ai_addr, iter->ai_addrlen)<0){	
					close(sockfd_local);
					printf("client: didnt connect\n");
					continue;
				}
				printf("pre break\n");
				break;
			}
	
			printf("Local is %d\n",sockfd_local);
			freeaddrinfo(res);
			*sockfd = sockfd_local;
			return (iter!=NULL); // returns 1 on success, 0 on failure
}

struct lab3message* parse_to_message(char*input_string){
	char *colon_token=":";
	char *comma_token = ",";
	int i =0;
	int counter = 0;
	int src_end=-1;
	int cmd_end=-1;
	for(i=0; i < strlen(input_string);i++){
		if (!counter && input_string[i] == ':'){
			src_end = i; 
			counter++;
		}
		else if (counter && input_string[i] == ':'){
			cmd_end = i;
			break;
		}

	}
	if (src_end < 0 || cmd_end<0){
		perror("Packet invalid");
		return NULL;
	}
	int size = strlen(input_string)-(cmd_end)-1/*this for colon delim*/;
	char *data = malloc(sizeof(char)*size+1);
	strncpy(data, input_string + (cmd_end+1),size);
	data[size] = '\0';
	
	char *source = strtok(input_string,colon_token);
	char *cmd=strtok(NULL, colon_token);
	 struct lab3message* ret = malloc(sizeof(struct lab3message));
	// for (i = 0; i < 13; i++){
	// 	if (!strcmp(cmd,type_dictionary[i])){
	// 		ret->type = i;
	// 		break;
	// 	}
	// }
	ret->type= atoi(cmd);
	ret->size = size;
	strcpy(ret->source,source);
	strcpy(ret->data,data);
	return ret;
}


int main(int argc, char *argv[]){

	printf("/login username password IP Port to login \n");
	char *input_string;
	input_string = (char *) malloc(sizeof(char)*MAX_LEN);
	int input_size=MAX_LEN;
	bzero(input_string, sizeof(char)*MAX_LEN);
	bool logged_in=false;
	bool in_conf=false;
	int cur_sess = -1;
	char*user= (char *) malloc(sizeof(char)*MAX_LEN);
	int max_sockfd;
	int base_socket=0;
	fd_set smain;
  	
  	
	while (1){

		printf("beginning\n");

		printf("Flags are log: %d conf: %d sessid: %d  \n",logged_in,in_conf,cur_sess);
		FD_ZERO(&smain);
		FD_SET(fileno(stdin),&smain);
		if (base_socket>0){
			FD_SET(base_socket,&smain);
			select((base_socket>fileno(stdin))?(base_socket+1):(fileno(stdin)+1),&smain,NULL,NULL,NULL);
		}
		else
			select(fileno(stdin)+1,&smain,NULL,NULL,NULL);


		if (base_socket >0 &&FD_ISSET(base_socket,&smain)){
			struct lab3message *retPacket;
			char buffer[MAX_LEN];
			bzero(buffer,sizeof(char)*MAX_LEN);
			recv(base_socket,buffer,MAX_LEN,0);
			printf("recv -%s-\n",buffer);

			retPacket=parse_to_message(buffer);
			if (retPacket==NULL){
				printf("Problem in packet\n");
				continue;
			}

			if(!strcmp(type_dictionary[retPacket->type],"LO_ACK")) {
				printf("login succeeded %s\n",user);
				logged_in = true;
				in_conf=false; 
				cur_sess = -1;
				//printf("login cpy %s\n",username);
				//strcpy(user,username);
				printf("login succeeded %s\n",user);

			}
			else if (!strcmp(type_dictionary[retPacket->type],"LO_NAK")){
				printf("failed login, retry");
				//FD_ZERO(&smain);
				base_socket = 0;
				//max_sockfd=0;
				logged_in=false;
				in_conf=false; 
				cur_sess = -1;
			}
			else if (!strcmp(type_dictionary[retPacket->type],"JN_ACK")){	
				printf("conf join succeeded\n");
				in_conf=true; 
				cur_sess = atoi(retPacket->data);
			}
			else if (!strcmp(type_dictionary[retPacket->type],"JN_NAK")){
				printf("Join failed due to",retPacket->data);
				in_conf=false; 
				cur_sess=-1;
			}
			else if(!strcmp(type_dictionary[retPacket->type],"IN_ACK")) {//PLACEHOLDER ARGUMENT
				printf("forced join succeeded\n");
			}
			else if (!strcmp(type_dictionary[retPacket->type],"IN_NAK")){//PLACEHOLDER ARGUMENT
				printf("cannot force them to join");
			}
			else if(!strcmp(type_dictionary[retPacket->type],"INVITE")){
				printf("Join session %s?",retPacket->data);
				char ans[MAX_LEN];
				char buffer[MAX_LEN];
				scanf("%s",ans);
				if (!strcmp(ans,"Yes")){
					sprintf(buffer, "%s:14:%s", user,retPacket->data);
					send(base_socket, buffer, MAX_LEN, 0);
				}
				else{
					sprintf(buffer, "%s:15:%s", user,retPacket->data);
					send(base_socket, buffer, MAX_LEN, 0);
				}

			}
			else if(!strcmp(type_dictionary[retPacket->type],"NS_ACK") /*&&  !strcmp(retPacket->data,sess_id)*/) {
				printf("conf create succeeded at %s\n",retPacket->data);
				in_conf=true; 
				cur_sess = atoi(retPacket->data);
			}
			else if(!strcmp(type_dictionary[retPacket->type],"QU_ACK")) {
				printf("Here are the current users and sessions: %s\n",retPacket->data);
			}
			else if(!strcmp(type_dictionary[retPacket->type],"MESSAGE")){
				printf("%s",retPacket->data);
			}
			free(retPacket);
		}
		else if(FD_ISSET(fileno(stdin),&smain)){
			char cmd[MAX_LEN];
			scanf("%s", cmd);
			printf("cmd is %s\n",cmd);
			printf("BAse sock is %d\n",base_socket);
			if (!strcmp(cmd,"/login")){
			
				char username[MAX_LEN], password[MAX_LEN], IP[MAX_LEN], port[MAX_LEN];
				if (!logged_in){
					scanf(" %s %s %s %s",username,password,IP,port);
					int connectsuccess=client_connect(IP, port, &base_socket);
					
					printf("Past connection with code of %d\n",connectsuccess);
					if(!connectsuccess){
						int base_socket=0;
						printf("CONNECTION FAILURE RETURN TO MENU\n");
						continue;
					}
					//FD_SET(base_socket, &smain);
					//max_sockfd=base_socket+1;

					char buffer[MAX_LEN];
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:0:%s,%s",username,username,password);
					puts(buffer);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}

					strcpy(user,username);
					
				}
				else{
					printf("Already logged in!\n");
				}
			}

			else if (!strcmp(cmd, "/logout")){
				
				if (logged_in){
					char buffer[MAX_LEN];
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:3:",user);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}
					close(base_socket);
					//FD_ZERO(&smain);
					base_socket = 0;
					
					logged_in = false;
					in_conf=false; 
					cur_sess=-1;
				}
				else{
					printf("Log in first before logging out");
				}
			}

			else if (!strcmp(cmd, "/joinsession")){

				if (logged_in && !in_conf){
					char buffer[MAX_LEN];
					char sess_id[MAX_LEN];
					scanf(" %s",sess_id);

					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:4:%s",user,sess_id);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}

								
				}
				else{
					printf("Log in before joining or leave current session");
				}
			}
			else if (!strcmp(cmd, "/invite")){

				if (logged_in){

					char buffer[MAX_LEN];
					char sess_id[MAX_LEN];
					char username[MAX_LEN];
					scanf(" %s %s",sess_id,username);
					
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:13:%s,%s",username,sess_id);

					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}

				}
				else{
					printf("Log in before joining or leave current session");
				}
			}
			else if (!strcmp(cmd, "/leavesession")){

				if (logged_in&&in_conf && cur_sess!=-1){
					char buffer[MAX_LEN];
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:7:",user);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}
					printf("successfully left session %d",cur_sess);
					in_conf = false;
					cur_sess = -1;
				}	
				else{
					printf("Not in a session\n");
				}
			}
			else if (!strcmp(cmd, "/createsession")){
				
				if (logged_in&&!in_conf){
					char buffer[MAX_LEN];
					char sess_id[MAX_LEN];
					scanf(" %s",sess_id);
					
					
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:8:%s",user,sess_id);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}
					puts(buffer);
					
					
				}
				else{
					printf("Not logged in or already in conference\n");
				}
			}
			else if (!strcmp(cmd, "/list")){
				if (logged_in){
					char buffer[MAX_LEN];
					bzero(buffer,sizeof(char)*MAX_LEN);
					sprintf(buffer,"%s:11:",user);
					if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
					}
					printf("querying...\n");

				}
				else{
					printf("Not logged in\n");
				}
			}
			else if (!strcmp(cmd, "/quit")){
				close(base_socket);
				exit(0);
			}
			else if(logged_in && in_conf&&cur_sess!=-1 &&strlen(cmd)!=0){
				char msg[MAX_LEN];
				strcpy(msg,cmd);

                if (!feof(stdin)){
                    gets(cmd);
                    strcat(msg,cmd);
                }
                printf("message: %s\n", msg);


				char buffer[MAX_LEN];
				bzero(buffer,sizeof(char)*MAX_LEN);

				sprintf(buffer,"%s:10:%s",user,msg);

				if (send(base_socket,buffer, MAX_LEN,0)==-1){
						perror("Sending fucked up");
				}
			}
			else{
				puts(cmd);
				printf("is not a valid input\n");
			}
		}

		
	}

}
			
		
