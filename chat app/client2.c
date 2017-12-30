#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

 

#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define MAX_NAME 10
#define MAX_DATA 100
#define MAX_FIELD 20

//======== Type specs ========//

struct packet {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_DATA];
	unsigned char msg[MAX_DATA];
};

char * type_dictionary[14] = {"LOGIN",
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
					  "INVITE"};
// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//======== helper functions ========//
void processPacket(struct packet data, char* packetInfo){
    
    sprintf(packetInfo, "%d:%d:%s:%s", data.type, data.size, data.source, data.msg);
    printf("%s\n", packetInfo);
    return;    
}

struct packet* parse_to_message(char*input_string){
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
	 struct packet* ret = malloc(sizeof(struct packet));
	// for (i = 0; i < 13; i++){
	// 	if (!strcmp(cmd,type_dictionary[i])){
	// 		ret->type = i;
	// 		break;
	// 	}
	// }
	ret->type= atoi(cmd);
	ret->size = size;
	strcpy(ret->source,source);
	strcpy(ret->msg,data);
	return ret;
}

void deProcessPacket(char * input_string){
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

int main(int argc, char *argv[]) {

    //======== Variable Setup ========//
    int sockfd;
    int numbytes;
    char buf[MAXDATASIZE];
    char message[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    struct packet data;
    struct packet reData;    

    char clientID[MAX_FIELD];
    char sessionID[MAX_FIELD];
    char password[MAX_FIELD];
    char serverIP[MAX_FIELD];
    char serverPort[MAX_FIELD];     
    char messageBuf[MAX_DATA];
    char sendBuf[MAX_DATA];

    int loggedIn = 0;

    fd_set readfds;
    

    //======== Conference Loop ========//
    while (1) {

        //======== select setup ========//
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin),&readfds);

        if (sockfd > 0) {
            FD_SET(sockfd, &readfds);
            select(sockfd+1,&readfds,NULL,NULL,NULL);    	
        }else{
            select(fileno(stdin)+1,&readfds,NULL,NULL,NULL);    	
        }

        //======== Responses to keyboard input ========//
        if (FD_ISSET(fileno(stdin),&readfds)){
            printf("input detected\n");  

            char command[MAX_DATA];
            scanf("%s", command);
            
            if (command[0] == '/'){
                printf("you entered a command: %s\n", command);
                
                if (!strcmp(command,"/login")){
                    
                    if (loggedIn){
                        printf("Already logged in\n");
                        int ch;
                        while ((ch = getchar()) != '\n' && ch != EOF);
                        continue;
                    }
                    else{
                        //======== input and packet processing ========//
                        scanf(" %s %s %s %s", clientID, password, serverIP, serverPort);
                        printf("clientID: %s\n", clientID);
                        printf("password: %s\n", password);
                        printf("serverIP: %s\n", serverIP);
                        printf("serverPort: %s\n", serverPort);

                        strcpy(data.msg,password);
                        data.type = 0;
                        data.size = strlen(password);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        char buffer[100];
						bzero(buffer,sizeof(char)*100);
						sprintf(buffer,"%s:0:%s,%s",clientID,clientID,password);
						printf("you entered: ");
						puts(buffer);
						

                        //======== Connection Setup ========//
                        memset(&hints, 0, sizeof hints);
                        hints.ai_family = AF_UNSPEC;
                        hints.ai_socktype = SOCK_STREAM;
                    
                        if ((rv = getaddrinfo(serverIP, serverPort, &hints, &servinfo)) != 0) {
                            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                            return 1;
                        }
                    
                        // loop through all the results and connect to the first we can
                        for (p = servinfo; p != NULL; p = p->ai_next) {
                            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                    p->ai_protocol)) == -1) {
                                perror("client: socket");
                                continue;
                            }
                    
                    
                            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                                close(sockfd);
                                perror("client: connect");
                                continue;
                            }
                    
                            break;
                        }
                    
                        if (p == NULL) {
                            fprintf(stderr, "client: failed to connect\n");
                            return 2;
                        }
                    
                        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
                                s, sizeof s);
                        printf("client: connecting to %s\n", s);

                        //======== login request ========//
                        //puts(sendBuf);
                        numbytes = send(sockfd,buffer,strlen(buffer),0);
                        printf("sendBytes: %d \n", numbytes);
                                                
                        continue;
                    }
                    
                }
                else if(!strcmp(command,"/logout")){

                    if (loggedIn){
                        close(sockfd);
                        loggedIn = 0;
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }

                }
                else if(!strcmp(command,"/joinsession")){

                    if (loggedIn){
                        //======== input and packet processing ========//
                        scanf("%s", sessionID);
                        
                        strcpy(data.msg,sessionID);
                        data.type = 4;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                    
                }
                else if(!strcmp(command,"/leavesession")){
                    if (loggedIn){
                        //======== input and packet processing ========//

                        strcpy(data.msg,"");
                        data.type = 7;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }                    
                }
                else if(!strcmp(command,"/createsession")){
                    if (loggedIn){
                        //======== input and packet processing ========//
                        scanf("%s", sessionID);
                        
                        strcpy(data.msg,sessionID);
                        data.type = 8;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }  
                }
                else if(!strcmp(command,"/list")){
                    if (loggedIn){
                        //======== input and packet processing ========//

                        strcpy(data.msg,"");
                        data.type = 11;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                }
                else if(!strcmp(command,"/quit")){
                    printf("bye!");
                    exit(0);
                }
                else if(!strcmp(command,"/invite")){
                    printf("invite command \n");
                    if (loggedIn){
                        char inviteID[MAX_FIELD];
                        
                        //======== input and packet processing ========//
                        scanf("%s", inviteID);
                        
                        strcpy(data.msg,inviteID);
                        data.type = 13;
                        data.size = strlen(inviteID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                    
                }

            }
            else{
                printf("you entered a message %s \n", command); 
                if (loggedIn){

                    strcpy(message,command);
                    if (!feof(stdin)){
                        gets(command);
                        strcat(message,command);
                    }
                    printf("message: %s\n", message);

                    //======== input and packet processing ========//

                    strcpy(data.msg,message);
                    data.type = 10;
                    data.size = strlen(data.msg);
                    strcpy(data.source, clientID);

                    processPacket(data,sendBuf);
                    
                    //======== request ========//
                    numbytes = send(sockfd, sendBuf, strlen(sendBuf), 0);
                    printf("numbytes: %d\n", numbytes);
                
                }
                else{
                    printf("you are not logged in!\n");
                }        
            }
        }//======== Responses to server replies ========//
        else if(FD_ISSET(sockfd,&readfds)){
            printf("Server sent something back\n");
            numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
            printf("numbytes: %d\n", numbytes);
            buf[numbytes] = '\0';            
            printf("client: received '%s'\n", buf);

            struct packet *reData=deProcessPacket(buf);
            // deProcessPacket(&reData.type,&reData.size, reData.source,	reData.msg, buf);
            // printf("type: %d\n", reData.type);
            // printf("size: %d\n", reData.size);
            // printf("source: %s\n", reData.source);
            // printf("msg: %s\n", reData.msg);

            switch (reData.type){   
                case(1):
                    printf("login successful!\n");
                    loggedIn = 1;
                    break;           
                case(2):
                    printf("login failed!\n");
                    break;        
                case(5):
                    printf("join successful!\n");
                    break;      
                case(6):
                    printf("join failed!\n");
                    break;      
                case(9):
                    printf("New Session Made!\n");
                    break;      
                case(10):
                    printf("%s: %s\n", reData.source, reData.msg);
                    break;        
                case(12):
                    printf("%s\n", reData.msg);
                    break;
                
                default:
                    printf("invalid reply\n");
            }
                

        }
        else{
            // printf("whats happening anymore\n");
        }
    }

    return 0;
}

