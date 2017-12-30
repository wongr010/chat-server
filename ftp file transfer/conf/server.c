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
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAXBUFLEN 100
#define SIZE_QUEUE 10
#define MAX_DATA 1000
#define MAX_LEN 100
#define MAX_USERNAME 50


/* To implement: 
- finish the switch case
- implement login checking in all functions
- login security?
*/



char * type_dictionary[16] = 
					  {"LOGIN",
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
struct lab3message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_LEN];
	unsigned char data[MAX_DATA];
};
struct user{
	char username[MAX_USERNAME];
	int sockfd;
	int conf_id;
	bool active;
};

struct conference{
	bool active;
	int users;
};

struct conference sessions[SIZE_QUEUE];

//Returns 0 on psuccess, otherwise fails
int LOGIN(struct user* userbase , char*user,char*pw, int sockfd){
	int i;
	for (i =0; i < SIZE_QUEUE;i++){
		if (!userbase[i].active)
		{
			userbase[i].sockfd = sockfd;
			strcpy(userbase[i].username,user);
			userbase[i].active = 1;
			userbase[i].conf_id = -1;
			return 0;
		}
	}
	return -1;
}

int LO_ACK(int sockfd){
	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:1:","server");
	// printf("Fisrt is %d\n",send(sockfd,buffer, 100,0));
	// puts(buffer);
	// printf("second is %d\n",send(sockfd,buffer, 100,0));
	// puts(buffer);
	return send(sockfd,buffer, MAXBUFLEN,0);
}

int IN_ACK(struct user *userbase, char *username, int sessionID, int sockfd){
	int i;
	for (i=0; i<SIZE_QUEUE; i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
		{								//user... not very efficient
			
			char minibuf[MAX_LEN];
			
			int err=LEAVE_SESS(userbase, userbase[i].sockfd);
			userbase[i].conf_id=sessionID; //force change

			sprintf(minibuf,"%d",sessionID);
			printf("JN RETURN: %d" ,JN_ACK(userbase[i].sockfd, minibuf));
			printf("force ok\n");
			return 0;
		
		}

	}
	
	//sprintf(buffer, "%s:14:", "server");
	

	//return send(sockfd, buffer, MAXBUFLEN, 0);
}

int INVITE(struct user *userbase, char *username, int sessionID,int sockfd){
	int i;
	
	for (i=0; i<SIZE_QUEUE; i++){
		if (!strcmp(userbase[i].username, username)) //right now looking through the entire array and finding the matching
		{								//user... not very efficient
			
			char minibuf[MAX_LEN];
			char temp[MAX_LEN];
			sprintf(temp, "%d", sessionID);
			sprintf(minibuf, "%s:13:%s", "server", temp);
			send(userbase[i].sockfd, minibuf, MAXBUFLEN, 0);
			
			
			
			break;
		}
	}
	
	
	return 0;
	

}

int IN_NAK(int sockfd){
	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:15:","server");
	printf("user or session ID nonexistent\n");
	return send(sockfd, buffer, MAXBUFLEN, 0);
}

int LO_NAK(int sockfd){
	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:2:","server");
	return send(sockfd, buffer, MAXBUFLEN, 0);
}

int JOIN(struct user* userbase, char *sessionID, int sockfd){
	int i;
	int ID=atoi(sessionID);


	if(ID<0 || ID>=SIZE_QUEUE){
		printf("Invalid session ID\n");
		return -1;
	}

	else if (sessions[ID].active==false){ //inactive session
		printf("This session hasn't been created yet \n");
		return -1;
	}


	for (i =0; i < SIZE_QUEUE;i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
		{								//user... not very efficient
			

			userbase[i].conf_id=ID;
			sessions[ID].users++; //one more user in this chat room
			return 0;
		}
	}
	printf("This client has not logged in yet\n");
	return -1;
}

int JN_ACK(int sockfd, char* sessionID){
	char buffer[MAX_LEN];
	
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:5:%s","server",sessionID);
	return send(sockfd, buffer, MAXBUFLEN, 0);
	

}


int JN_NAK(int sockfd){
	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:6:","server");
	return send(sockfd, buffer, MAXBUFLEN, 0);
	
}

int EXIT(struct user *userbase, int sockfd){
	int i;
	for (i =0; i < SIZE_QUEUE;i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
			{								//user... not very efficient

			if (userbase[i].conf_id!=-1){
				if (!LEAVE_SESS(userbase, sockfd)){ //leave any active sessions if the user is in any
					printf("error in leaving sessions while exiting \n");
					return -1;
				}
			}
				userbase[i].active=false; //do we need to anything else to exit?

				return 0;
			
		}
	}
	printf("This client has not logged in yet\n");
	return -1;
}

int LEAVE_SESS(struct user *userbase, int sockfd){ //right now each user can only be in 1 session

	int i;
	for(i=0; i<SIZE_QUEUE; i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
		{								//user... not very efficient

			sessions[userbase[i].conf_id].users--;

			if (sessions[userbase[i].conf_id].users==0) //no one in this session, session no longer active
				sessions[userbase[i].conf_id].active=false; 


			userbase[i].conf_id=-1; 
			return 0;
		}
	}

	return -1;

}

int NEW_SESS(struct user *userbase, int sockfd, int *newID){
	int i;
	for (i =0; i < SIZE_QUEUE;i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
		{								//user... not very efficient
			
			printf("found\n");
			int j;
			for (j=0; j<SIZE_QUEUE; j++){
				if(!sessions[j].active){
					userbase[i].conf_id=j; //assign this conference to this user
					printf("new user made conference %d\n", j);
					sessions[j].users++; //one more user in this chat room
					sessions[j].active=true; //room activated
					*newID=j;
					break;

				}
			}

			if (j==SIZE_QUEUE){
				printf("all available sessions taken\n"); //theoretically should not happen

				return -1;
			}
			
			return 0; //RETURN 0 UPON SUCCESS
		}
	}

	return -1;
}


int NS_ACK(char* sessionID, int sockfd){

	

	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:9:%s","server", sessionID);
	puts(buffer);
	printf("Sending to sock %d\n",sockfd);
	int ers=send(sockfd, buffer, MAXBUFLEN, 0);
	printf("Sent it!\n");
	return ers;
	
	//return send(sockfd, message, MAXBUFLEN, 0); //can do this?
}

int MESSAGE(struct user* userbase, char *message, int sockfd){
	puts(message); //print out this message
	int currconf;
	char buffer[MAX_LEN];
	bzero(buffer,sizeof(char)*MAX_LEN);
	sprintf(buffer,"%s:10:%s","server", message);
	int i;
	for (i=0; i<SIZE_QUEUE; i++){
		if (userbase[i].sockfd==sockfd) //right now looking through the entire array and finding the matching
		{								//user... not very efficient
			currconf=userbase[i].conf_id;

			
			break;
		}
	}

	for (i=0; i<SIZE_QUEUE; i++){
		//very brute force method 
		if(currconf==userbase[i].conf_id){
		int err=send(userbase[i].sockfd, buffer, MAXBUFLEN, 0);
		if (err==-1) {//error code is -1 I believe
			printf("could not send to user %d", i);
			return -1;
		}
	}
	}

	return 0;
}

int QU_ACK(struct  user* userbase, int sockfd){
	char users[MAXBUFLEN]; //created on the stack, not sure if better to malloc. This is the main message
	char separator[4];
	char newline[2];
	char sessionlist[10];
	//char sessionID[2];
	char temparray[3];
	strcpy(users, "Users: ");
	strcpy(separator, " | ");
	strcpy(newline, "\n");
	strcpy(sessionlist, "Sessions: ");
	int i;
	for (i=0; i<SIZE_QUEUE; i++){
			if(userbase[i].active){
			strcat(users, userbase[i].username);
			strcat(users, separator);
		}
	}

	strcat(users, newline);
	strcat(users, sessionlist);
	for (i=0; i<SIZE_QUEUE; i++){
		if(sessions[i].active){
			sprintf(temparray, "%ld", i); //can do thi?
			//strcpy(sessionID, temparray; //can do int to string?
			strcat(users, temparray);
			strcat(users, separator);
		}
	}
	char buffer[MAXBUFLEN];
	bzero(buffer,sizeof(char)*MAXBUFLEN);
	sprintf(buffer,"%s:12:%s","server", users);
	if (send(sockfd, buffer, MAXBUFLEN, 0)==-1){
		printf("quack didnt work\n");
	}
	
	return 0;
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

	int base_socket, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	fd_set smain;
  	fd_set copy;
  	FD_ZERO(&smain);
  	FD_ZERO(&copy);

   // char s[INET6_ADDRSTRLEN];
	int rv;

	//initialize the conference table
	int k;
	for (k=0; k<SIZE_QUEUE; k++){ //the array id represents the session ID
		sessions[k].active=false;
		sessions[k].users=0;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((base_socket = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		// if (setsockopt(base_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
		// 		sizeof(int)) == -1) {
		// 	perror("setsockopt");
		// 	exit(1);
		// }
		 if (bind(base_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(base_socket);
			perror("server: bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo); // all done with this structure


	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(base_socket, SIZE_QUEUE) == -1) {
		perror("listen");
		exit(1);
	}
	FD_SET(base_socket, &smain);
	int max_sockfd=base_socket+1;

	printf("server: waiting for connections...\n");

	
  	
  	


  	struct user* userbase = malloc(SIZE_QUEUE*sizeof(struct user));
  	memset(userbase,0,SIZE_QUEUE*sizeof(struct user));


	
	int sockFD;
	while(1) {  // main accept() loop
		copy=smain;
		sin_size = sizeof their_addr;
	
		//FD_ZERO(&smain);
  		//FD_SET(base_socket, &smain);
		//FD_ISSET(base_socket,&smain);
		int status=select(max_sockfd, &copy, NULL, NULL, NULL);

		if (!status)
			continue;
		else if (status == -1){
			perror("Select fucked up");
			break;
		}

		//New connection
		if (FD_ISSET(base_socket,&copy)){
			int new_fd = accept(base_socket, (struct sockaddr *)&their_addr, &sin_size);
			//perror("Socket unaccpetable");
			printf("new fd is %d\n",new_fd);
			FD_SET(new_fd,&smain);
			max_sockfd ++;
		}
		//Handle client commands
		else{
			int i;
			for (i =base_socket; i<= max_sockfd; i ++){
				if (FD_ISSET(i,&copy)){//Implies new data, we need to parse
					struct lab3message* msg;
					char *buffer = malloc (sizeof(char) * MAXBUFLEN);
					recv(i,buffer,MAXBUFLEN,0);
					msg = parse_to_message(buffer);
					puts(type_dictionary[msg->type]);

					switch(msg->type){
						case 0:
							printf("login");
							if (!LOGIN(userbase,strtok(msg->data,","),strtok(NULL,","),i)){
								if (LO_ACK(i)==-1){
									perror("Login ack for user failed, crashing server");
									exit(0);
								}
							}
							else{
								printf("Failed to login\n");
								if(LO_NAK(i)==-1){
									perror("could not send NAK to sockfd \n");
									exit(0);
								}
							}
						break;

						case 4: //join
							printf("join\n");
							if(!JOIN(userbase, msg->data,i)){
								if (JN_ACK(i, msg->data)==-1){
									perror("Could not send join ack\n");
									exit(0);
								}
							}

							else{
								printf("Failed to join session\n");
								if (JN_NAK(i)==-1){
										perror("Could not send jn nak\n");
										exit(0);
								}
							}

						break;

						case 3: //exit	
							printf("exit\n");
							if(EXIT(userbase, i)){
								perror("Could not exit \n");
							}

						break;

						case 7: //leave sess

							printf ("leave session\n");
							if(
								LEAVE_SESS(userbase, i)){
								printf("failed to leave sess\n");
							}

						break;

						case 8: //new sess
							printf("creating new session \n");
							int new_id;
							if (NEW_SESS(userbase, i, &new_id)){
								printf("could not create new sess\n");
							}
							char stringID[4];
							sprintf(stringID, "%ld", new_id);

							//char *stringID=itoa(new_id); //idk if this is correct syntax
							if (NS_ACK(stringID, i)==-1){
								printf("failed to ack new sesssin\n");
							}
						break;

						case 10: //message
							printf("new messgage\n");
							if (MESSAGE(userbase, msg->data, i)){
								printf("message send failed\n");
							}

						break;

						case 11: //query
							printf("Querying\n");
							int err=QU_ACK(userbase, i); //did not write any error codes for this
						break;

						default:
							printf("unlucky\n");
						break;
					

					case 13: //invite
						printf("invite to join\n");

						 INVITE(userbase, strtok(msg->data, ","), atoi(strtok(NULL,"\0")),i);
						//send(i, "yes", MAXBUFLEN, 0);
					break;

					case 14: //invite
						printf("in ack\n");

						 IN_ACK(userbase, strtok(msg->data, ","), atoi(strtok(NULL,"\0")),i);
						//send(i, "yes", MAXBUFLEN, 0);
					break;

					case 15:
						printf("in nack\n");
						IN_NAK(i);
						break;
				}


					free(msg);
					free(buffer);
				}
			}
		}
	
	
	}
	return 0;
}


