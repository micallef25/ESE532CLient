#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>

#define PORT 8091
#define CHUNKSIZE 4096
#define HEADER 2
#define DONE_BIT (1 << 7)


void handle_input(int argc, char* argv[],int* sleep_time, char** ip, char** filename)
{
	int c;
	extern char *optarg;
	extern int optind, optopt, opterr;


	while ((c = getopt(argc, argv, ":s:f:i:")) != -1)
	{
		switch(c)
		{
		case 's':
		    *sleep_time = atoi(optarg);
			printf("sleep is set to %d optarg\n",*sleep_time);
			break;
		case 'i':
		    *ip = optarg;
			printf("ip is set to %s\n",*ip);
			break;
		case 'f':
			*filename = optarg;
        	printf("filename is %s\n", *filename);
        	break;
    	case ':':
        	printf("-%c without parameter\n", optopt);
        	break;
    	}
	}
}

int main(int argc, char* argv[] )
{
  
	char* file = "vmlinuz.tar";
	char* ip_addr = "192.168.0.100";
	int sleep_time = 5;
    handle_input(argc,argv,&sleep_time,&ip_addr,&file);

    printf("sleep is set to %d optarg\n",sleep_time);
    printf("ip is set to %s\n",ip_addr);
    printf("filename is %s\n", file);



  	//
	FILE* fp = fopen(file,"r" );
	if(fp == NULL ){
		perror("invalid file");
		exit(EXIT_FAILURE);
	}
		
	//
	fseek(fp, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp); // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

	//
	unsigned char* buff = (unsigned char*)(malloc(sizeof(unsigned char) * file_size ));	
	if(buff == NULL)
	{
		perror("not enough space");
		fclose(fp);
		exit(EXIT_FAILURE);
	}	
	
	//	
	int bytes_read = fread(&buff[0],sizeof(unsigned char),file_size,fp);
	printf("bytes_read %d\n",bytes_read);

	//
	int remainder = bytes_read % CHUNKSIZE;
	int loops = bytes_read / CHUNKSIZE;

	//printf("loops %d remainder %d\n",loops,remainder);

	//
	int sockfd; 
    struct sockaddr_in servaddr; 
	int opt = 1;  
    
	// Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	{ 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr));

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt)))
	{
       perror("sockopt");
       exit(EXIT_FAILURE); 	
	}    

	bzero(&servaddr,sizeof(servaddr)); 
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = htons(ip_addr);

    if(inet_pton(AF_INET,ip_addr,(&(servaddr.sin_addr))) < 0)
    	perror("inet");

    //
    int n;

    sleep(10);

    //
    unsigned char local[CHUNKSIZE+HEADER];

    // send chunk size bytes
	for( n = 0; n < loops-1; n++)
	{
		//
		memcpy(&local[HEADER],&buff[n*CHUNKSIZE],CHUNKSIZE);

		// 
		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		//
		local[0] = low;
		local[1] = high;

		// sleep for a bit you will likely need to sleep if you are running client and server on one board
		if(sleep_time)
			usleep(sleep_time);

		//
		sendto(sockfd,&local[0],CHUNKSIZE+HEADER,0,(const struct sockaddr *) &servaddr,sizeof(servaddr)); 
	}

	// if no remainder send the last chunk with done bit
	if(remainder == 0){
		
		//
		memcpy(&local[HEADER],&buff[n*CHUNKSIZE],CHUNKSIZE);

		//
		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		//
		local[0] = low;
		local[1] = high | DONE_BIT;


		sendto(sockfd,&local[0],CHUNKSIZE+HEADER,0,(const struct sockaddr *) &servaddr,sizeof(servaddr)); 
		n++;
	}
	// if remainder then send last chunk as normal
	else{
		
		//
		memcpy(&local[HEADER],&buff[n*CHUNKSIZE],CHUNKSIZE);

		//
		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		//
		local[0] = low;
		local[1] = high;

		//
		sendto(sockfd,&local[0],CHUNKSIZE+HEADER,0,(const struct sockaddr *) &servaddr,sizeof(servaddr)); 

		n++;
	}

	// send remaining pieces
	if(remainder){
		
		//
		memcpy(&local[HEADER],&buff[loops*CHUNKSIZE],remainder);

		//
		char high = remainder >> 8;
		char low = remainder & 0xFF;

		//
		local[0] = low;
		local[1] = high | DONE_BIT;


		sendto(sockfd,&local[0],CHUNKSIZE+HEADER,0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
		n++; 
	}

	//printf("test complete sent about %d packets\n",n);
	sleep(10);
	close(sockfd);
	fclose(fp);
	free(buff); 
    return 0; 

}
