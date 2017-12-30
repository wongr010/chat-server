/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: wongros4
 *
 * Created on September 28, 2017, 9:20 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 10000


void msgToPacket(int *total, int *n, int * size, int*name, int*msg, char* rawmessage, int bytesize){
	int i = 0;
	int counter = 0;
	*total = 0;
	for (i = 0 ; i < bytesize; i++){
		if (rawmessage[i] == ':'){	
			counter ++;
			switch(counter){
			case 1:
				*n = i+1;
			break;
			case 2:
				*size = i+1;
			break;
			case 3:
				*name = i+1;
			break;
			case 4:
				*msg = i+1;
			break;
				
			}			
			
			
		}
		
		
		if (counter == 4)
			break;
	}

}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int string2int(char * num){
    int dec = 0, i, j, len;
    len = strlen(num);
	for(i=0; i<len; i++){
		dec = dec * 10 + ( num[i] - '0' );
	}
    return dec;
}
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	srand(time(NULL));
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
                if (argc != 2) {
                fprintf(stderr,"usage: server port\n");
                    exit(1);
                    }    
/************************************/  
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}


    /***************************/



	printf("listener: waiting to recvfrom...\n");
                
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);
        char * response1 = '\0';
        if (strcmp(buf,"ftp") == 0){
           response1 = "yes";
        }
        else
                response1 = "no";

  
         if (numbytes=sendto(sockfd,response1,strlen(response1),0,(struct sockaddr*) &their_addr, sizeof their_addr) == -1){
                perror ("server: sendto");
                exit(1);
            }
        printf("sent %s to thingy\n",response1);
		
		char *compiledPacket; 
		if (!strcmp("yes",response1)){

			if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}
				//send ack

int r = rand()%6+1;
printf("\nslept for %d\n",r);
sleep(r);
				char * ack="acknowledged\0";
				if (sendto(sockfd,ack,strlen(ack),0,(struct sockaddr*) &their_addr, sizeof their_addr) == -1){
                perror ("server: acknowledged");
                exit(1);
            }
			int counter = -1;
			int total,n,size,name,msg;
 			msgToPacket(&total,&n,&size,&name,&msg,buf,numbytes);
			int i_ = 0;
			
			char subbuff[numbytes-msg];
			memcpy( subbuff, buf[msg], numbytes-msg );
			printf("%s",subbuff);

			for (i_ = name; i_ < msg-1; i_++){
				printf("%c",buf[i_]);
			}
			FILE *rote;
			
   			rote = fopen("./hospit.png", "w+");
	fputs(subbuff,rote);
fclose(rote);
			/*do{

				
				if (counter == -1){
					msgToPacket(&total,&n,&size,&name,&msg,buf,numbytes);
						

					char totalNumber[n-total];
					memcpy( totalNumber, &buf[total], n-total-1 );
					totalNumber[n-total] = '\0';
					compiledPacket = (char *) malloc(sizeof(char) * 1000* string2int(totalNumber));					
					counter = string2int(totalNumber);
					
				}
				char packNumber[size-n];
				memcpy( packNumber, &buf[n], size-n-1 );
				packNumber[size-n] = '\0';
				char messageSize[name-size];
				memcpy( messageSize, &buf[size], name-size-1 );
				messageSize[name-size] = '\0';
				int j = 0;
				for (j = msg; j< numbytes; j++)
					compiledPacket[string2int(packNumber)*1000 + j] = buf[msg+j];
				printf("listener: packet is %d bytes long\n", numbytes);
				buf[numbytes] = '\0';
				printf("listener: packet contains \"%s\"\n", buf);	

				counter--;
				}
				while(counter >0);
				
				printf("Final is %s", compiledPacket);*/
			}
//free (compiledPacket);
freeaddrinfo(servinfo);
close(sockfd);

	return 0;
}


