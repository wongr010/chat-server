#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#define SERVERPORT "4950" // the port users will be connecting to
#define MAXBUFLEN 100
#define MAXTIME 3

/*static int ftp(){
    
}
static struct{
    const char *name;
    int (*func) (int nargs, char **args);
}cmdtable[]{
    {"ftp", ftp}
};
*/

struct packet{
    unsigned int total_frag; 
unsigned int frag_no; 
unsigned int size;
char* filename;
char filedata[1000];
};


int exists( char *fname){
    FILE * file;
  
    if (file=fopen(fname, "r")){
        fclose(file);
        return 1;
    }
    else return 0;
    }

char * packetToMessage(struct packet pack, int * msglen){
    
	int frag_t = (int)((floor(log10(pack.total_frag)+1)));
        
        
                    
	int frag_n = (int)((floor(log10(pack.frag_no)+1)));
        if (pack.frag_no == 0)
            frag_n = 1;
        
	int size = 	(int)((floor(log10(pack.size) + 1)));
        
	int name = strlen(pack.filename);

        //printf("%d %d %d %d",frag_t,frag_n,size, name);
         
	char buffer1[frag_t+1];
	char buffer2[frag_n+1];
	char buffer3[size+1];

	sprintf(buffer1, "%u", pack.total_frag);
	sprintf(buffer2, "%u", pack.frag_no);
	sprintf(buffer3, "%u", pack.size);
	int finalSize = pack.size + name + size + frag_n + frag_t;
	char *message = (char *) malloc(sizeof(char) * finalSize + 4);
	int i = 0;
	int curIdx=0;
	
	*msglen = finalSize + 4;


	for (i = curIdx; i < frag_t; i++){
		message[i]= buffer1[i];
	}
	message[curIdx+frag_t] = ':';
	curIdx = curIdx + frag_t + 1;


	for (i = 0; i < frag_n; i++){
		message[curIdx + i]= buffer2[i];
	}
	message[curIdx +frag_n] = ':';
	curIdx = curIdx + frag_n + 1;


	for (i = 0; i < size; i++){
		message[curIdx + i]= buffer3[i];
	}
	message[curIdx +size] = ':';
	curIdx = curIdx + size + 1;

	for (i = 0; i < strlen(pack.filename); i++){
		message[curIdx + i]= pack.filename[i];
	}
	message[curIdx +strlen(pack.filename)] = ':';
	curIdx = curIdx + strlen(pack.filename) + 1;
    
    for (i = 0; i < pack.size; i++){
		message[curIdx + i]= pack.filedata[i];
	}
	curIdx = curIdx + pack.size;
                  //char *message = (char *) malloc(sizeof(char) * 100 + 4);
	return message;
}

int filesize(char *fname){
    FILE * file;
    file=fopen(fname, "r");
    fseek(file, 0, SEEK_END);
    
    int size=ftell(file);
    printf("file size: %d \n", size);
    fclose(file);
    return size;
    
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
    
     if (argc != 3) {
    fprintf(stderr,"usage: talker hostname port\n");
    exit(1);
 }
 int sockfd;

 struct addrinfo hints, *servinfo, *p;
 int rv;
 int numbytes;
 int i;
 char buf[MAXBUFLEN];
 struct sockaddr_storage their_addr;
 char* message = "\0";
// for (i =0; i < argc;i++){
 //    printf("%s", argv[i]);
 //}


 char* ftp;
char filename[100];
//filename[99] = '\0';
 printf("ftp filename: \n");
//fgets(filename, 25, stdin);
 scanf("%s", &filename);

 if (exists((filename))){
     message = "ftp";
 }
 else 
     exit(1);
 
 
/************/

char* talker, hostname;


 memset(&hints, 0, sizeof hints);
 hints.ai_family = AF_UNSPEC;
 hints.ai_socktype = SOCK_DGRAM;
 if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

     return 1;
 }
 // loop through all the results and make a socket
 for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
    p->ai_protocol)) == -1) {
        perror("talker: socket");
        continue;
    }
 break;
 }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
/*************/




 time_t sent = time(0);
 if ((numbytes = sendto(sockfd, message, strlen(message), 0,
 p->ai_addr, p->ai_addrlen)) == -1) {
 perror("talker: sendto");
 exit(1);
 }

 //printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
 //printf("talker: pre var init");
 socklen_t addr_len;

 //printf("talker: var init");
if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		p->ai_addr, &(p->ai_addrlen))) == -1) {
printf("talker: shits fucking up");		
    perror("recvfrom");
		exit(1);
	}
 //sleep(1);
 time_t recieved = time(0);

 buf[numbytes] = '\0';
 printf("talker: packet contains \"%s\" which took %f seconds\n", buf, difftime(recieved, sent));
 










if (!strcmp(buf,"yes")){
 int fsize=filesize(filename);
 int numpackets=fsize/1000;
 if (fsize>numpackets*1000) numpackets++;
 	printf("Number of fragments %d \n", numpackets);
 int j;
 FILE *file = fopen(filename, "r");
   if (file == NULL){
        printf("file not found"); //could not open file
        exit(2);
    }
 for (int j=0; j<numpackets; j++){
     struct packet a;
     a.total_frag=numpackets;
     a.frag_no=j;
     printf("fragment number is %d \n", a.frag_no);
     a.filename=filename;
    
    
   // char code[1000];
    int n = 0;
    int c;
    
  

  
    while(n<1000){
        c=fgetc(file);
        if (c==EOF) break;
        
        a.filedata[n]=c;
        //printf("%c", a.filedata[n]);
        n++;
    }
    
    //a.filedata[n]='\0';
        
   
 
    a.size=n;
      
  printf("\n%u size \n", a.size);
 // printf("%s",a.filename);
     //convert this packet to a string and send
	int msglen = 0;
 char * packetMsg = packetToMessage(a,&msglen);
    if ((numbytes = sendto(sockfd, packetMsg, msglen, 0,p->ai_addr, p->ai_addrlen)) == -1) {
 		perror("talker: sendto");
 		exit(1);
 	}
	 time_t TO_sent = time(0);
	time_t TO_received=time(0);

do{	
	numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,p->ai_addr, &(p->ai_addrlen)); 
	
	printf("%f",difftime(TO_received, TO_sent));
	TO_received=time(0);
	if (difftime(TO_received, TO_sent)> MAXTIME){
		printf("resending");
		sendto(sockfd, packetMsg, msglen, 0,p->ai_addr, p->ai_addrlen);
		TO_sent=time(0);
	}
}while (numbytes<=0);
	printf("\nTook %f s for reponse\n",difftime(TO_received, TO_sent));
	 if (numbytes == -1){
		printf("ack didn't work");
    	perror("recvfrom");
		exit(1);
	}
	 if (strcmp(buf,"acknowledged") == 0){
           printf("\nGot the ack");
        }

    //int i = 0;
    //for(i = 0; i < msglen; i++){
	//	printf("%c",packetMsg[i]);	
	//}
    free(packetMsg);
 }
  fclose(file);
}

freeaddrinfo(servinfo);

 //printf("Recieved %s", buf);
 close(sockfd);
 return 0;
}
