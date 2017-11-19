/*
** listener.c -- a datagram sockets "server" demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include  <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>




#include <netdb.h>
//#define MYPORT "9130"   // the port users will be connecting to
#define MAXBUFLEN 520

int nextchar =-1;

 /*opcode  operation
            1     Read request (RRQ)
            2     Write request (WRQ)
            3     Data (DATA)
            4     Acknowledgment (ACK)
            5     Error (ERROR)
Format specified by the IETF RFC 1350 FORMAT
*/
#define RRQ 1
#define WRQ 2
#define Data 3
#define ACK 4
#define ERROR 5


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void sigchld_handler(int signum)
{
    int saved_errno = errno;

    while (waitpid( -1, NULL, WNOHANG) > 0) ;
        
    errno = saved_errno;
}

//a function to read 512 or less bytes from the file indicated by file pointer
ssize_t readfromfile(FILE *fp,char *ptr,uint16_t block_num,char *sendbuf) {
	memset(sendbuf,0,strlen(sendbuf));
	ptr=sendbuf;
	//printf("block_num=%d\n",block_num);
	*ptr=0x00;
	ptr++;
	*ptr=0x03;
	ptr++;
	
	if(block_num<=255) {
	*ptr=0x00;
	ptr++;
	*ptr=block_num;
	ptr++;
	}
	else {
	*ptr=((block_num)&(0xFF00))>>8;
	ptr++;
	*ptr=(block_num)&(0x00FF);
	ptr++;
	}

	//printf("block again : %d\n",sendbuf[3]);
	char c;
	int count;
	for(count=0;count<512;count++) {
		if(nextchar>=0) {
			*ptr++ =nextchar;
			nextchar=-1;
			continue;
		}
		c=getc(fp);
		
		if(c==EOF) {
			if(ferror(fp))
				printf("read err from getc on local file\n");
			return(count+4);
		}

		else if(c=='\n') {
			c='\r';
			nextchar='\n';
		} else if(c=='\r') {
			nextchar='\0';
		}
		else
			nextchar=-1;
		*ptr++=c;
	}
		count=516;
		
	//printf("%c\n",sendbuf[4]);
	//printf("%c\n",sendbuf[5]);
	//printf("count=%d\n",count);
  return count;
}

ssize_t write_to_file(FILE *my_file,char *ptr4,int recvbytes,char *recv_buffer) {
ptr4=recv_buffer;
ptr4=ptr4+4;

char c,ch;
int count;
for(count=0;count<recvbytes-4;count++) {
	c=*ptr4;
	ptr4++;
	ch=*ptr4;
	if((c=='\r' )&& (ch=='\n') ) {
		putc(c,my_file);

	}

	else if((c=='\r') && (ch=='\0')) { 
		putc(c,my_file);

	}

	else {

	putc(c,my_file);

	}

}

return count;

	   
		
	


}
	



int readable_timeo(int fd,int sec) { 	
	fd_set rset;
	struct timeval tv;
	
	FD_ZERO(&rset);
	FD_SET(fd,&rset);
	
	tv.tv_sec= sec;
	tv.tv_usec=0;
	return (select(fd+1,&rset,NULL,NULL,&tv));
		/* >0 if descriptor is readable */

}

int main(int argc,char *argv[])
{
    int sockfd,child_socket;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int yes=1;
    int tv=0;
    int sendbytes,recvbytes;
    struct sigaction sa;
    int opcode;
    unsigned char send_buffer[520];
    unsigned char recv_buffer[MAXBUFLEN];
    int numbytes;
    struct sockaddr_storage their_addr;
     struct sockaddr_in servaddr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

        if (argc != 3) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
  //  hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
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
    freeaddrinfo(servinfo);
    printf("listener: waiting to recvfrom...\n");

	while(1) {
	    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    

	if(!fork()) {

	opcode=buf[1];
	


	//printf("The opcode for the packet is : %d\n",opcode);  
       sa.sa_handler = sigchld_handler; // reap all dead processes
       sigemptyset(&sa.sa_mask);
       sa.sa_flags = SA_RESTART;
       if (sigaction(SIGCHLD, &sa, NULL) == -1)
       {
        perror("sigaction");
        exit(1);
       }


	 
            servaddr.sin_port = htons(0);  // For ephemeral port
            
             /* Creatinf socket in child process */
                 if ((child_socket = socket(AF_INET,SOCK_DGRAM,0)) == -1)
                 {
                      perror("server_child_socket error");
                      exit(1);
                 }
                 if (setsockopt(child_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
                 {
                     perror("server_child_setsockopt error");
                     exit(1);
                 }
                 if (bind(child_socket, (struct sockaddr *)&servaddr, sizeof servaddr) == -1)
                 {
                     close(child_socket);
                     perror("server_child_bind error");
                     exit(1);
                 }
	
	
		//printf("I am here\n");
		int i=0;
		int count;
		char filename[30];
		char mode[10];
		
		strcpy(filename,&buf[2]);
		filename[strlen(filename)]='\0';
		strcpy(mode,&buf[3+strlen(filename)]);
		mode[strlen(mode)]='\0';
		
		
		
		//printf("%s\n",mode);
		//printf("%s\n",filename);
	if(opcode==RRQ) {
		if(!strcmp(mode,"netascii")) {
		//	printf("I am here\n");
			FILE *fp;
			fp=fopen(filename,"r");
			uint16_t block_num=1;
			char *ptr2;
			while((count=readfromfile(fp,ptr2,block_num,send_buffer))<=516) { 
					if(count==0) break;
					
					
		//			printf("count:%d\n",count);
				  int timeout_counter=0;
			up:	  if((sendbytes=sendto(child_socket,send_buffer,count,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
                           		 break;
				   }
					//printf("bytes sent are :%d\n",sendbytes);
			           while((tv<=0 || tv==1) && (timeout_counter<=10)) {
					//printf("Inside the time val loop\n");
				   	if((tv=readable_timeo(child_socket,1))==0) {
					 	printf("Timeout has occured\n");
					  	timeout_counter++;
						goto up;
						
				    	}
				
				  	else if (tv==-1) {
						printf("select and timeout error\n");
				    	}
				
				   	else {
					printf("The socket descriptor is ready \n");
					break;
				    	}
				   }

				if(timeout_counter>10) {
							printf("The timeout counter has exceeded 10\n");
							fclose(fp);
							close(child_socket);
							exit(1);
						}
				
				memset(recv_buffer,0,strlen(recv_buffer));
				if((recvbytes= recvfrom(child_socket, recv_buffer, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len))==-1) {
					perror("server_child_recvfrom error");
						break;
					}
				else {
					if(recv_buffer[1]==ACK) {
						uint16_t block_num_from_packet=(recv_buffer[2]<<8)|(recv_buffer[3]);
					//	printf("block num from packet:%d\n",block_num_from_packet);						
						if(block_num_from_packet==block_num)  {
							printf("Packet with block number %d has been sent and acknowledged\n",block_num);
							block_num++;
							tv=0;
						}
						else {
							goto up;
							tv=0;
						}
					}
				 }
															
					//printf("recvbytes=%d\n",recvbytes);

				if(count>=0 && count<516) {
					//printf("PAcket with block number %d has been sent and acknowledged\n",block_num);
					printf("Transfer complete\n");
					break;
				}

					
									
                           } //while

		}  //mode netascii closed

				
		if(!strcmp(mode,"octet")) {
			ssize_t read_bytes;
			int fd;
			fd=open(filename,O_RDONLY);
			char *ptr3;
			uint16_t block_sequence=1;
			ptr3=send_buffer;
			
	

		there:	memset(send_buffer,0,strlen(send_buffer));
			ptr3=send_buffer;
			*ptr3=0x00;
			ptr3++;
			*ptr3=0x03;
			ptr3++;
			
			if(block_sequence<=255) {
		
				*ptr3=0x00;
				ptr3++;
				*ptr3=block_sequence;
				ptr3++;
			}
			else {
				*ptr3=((block_sequence)&(0xFF00))>>8;
				ptr3++;
				*ptr3=(block_sequence)&(0x00FF);
				ptr3++;
			 }
			//printf("BLOCK NUMBER SENT : %u\n",block_sequence);
			//printf("%d\n",atoi(&send_buffer[2]));
			read_bytes=read(fd,ptr3,512);
				if((sendbytes=sendto(child_socket,send_buffer,read_bytes+4,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
				   }

			 if((read_bytes<512) && (read_bytes>=0))  { 
				//This is the last packet 
				printf("Packet with block number %u has been sent and acknowledged\n",block_sequence);
				printf("Data bytes sent : %lu\n",read_bytes);
				 printf("FIle transfer complete\n");
			 }
			 else if (read_bytes==-1) 
				printf("Error in read \n");
			
			else {
				memset(recv_buffer,0,strlen(recv_buffer));
				if((recvbytes= recvfrom(child_socket, recv_buffer, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len))==-1) {
					perror("server_child_recvfrom error");
					
					}
				else {
					if(recv_buffer[1]==ACK) {
						uint16_t block_num_from_packet=(recv_buffer[2]<<8)|(recv_buffer[3]);
					//	printf("block num from packet:%u\n",block_num_from_packet);						
						if(block_num_from_packet==block_sequence)  {
							printf("Packet with block number %u has been sent and acknowledged\n",block_sequence);
							printf("Data bytes sent : %lu\n",read_bytes);
							block_sequence++;
					//		printf("This is the print for block number:%u\n",block_sequence);
					
						}
						goto there;
					}
				  
				}
			}
		}  //IF mode is octet bracket closed
			
				
			
					
					
	
		
		close(child_socket);

		        
		 

	}// RRQ IF closed

		if(opcode==WRQ) {
		
		if(!strcasecmp(mode,"octet")) {
			
			char *ptr4;
			char *ptr5;
			unsigned char error_message[520];
			memset(error_message,0,strlen(error_message));
			uint16_t expected_block_num=1;
			uint16_t block_n;
			char my_str[25]="File already exists";
			int len=strlen(my_str);
			//printf("len=%d\n",len);
			ptr4=send_buffer;
			*ptr4=0x00;
			ptr4++;
			*ptr4=ACK;
			ptr4++;
			*ptr4=0x00;
			ptr4++;
			*ptr4=0x00;
			ptr4++;

			if((sendbytes=sendto(child_socket,send_buffer,4,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
				   }
			int fd1;
			fd1=open(filename,O_WRONLY|O_CREAT|O_EXCL, 0644);
			if(fd1==-1) {
				if(errno==EEXIST) {
					printf("[ERROR] ! FIle already exists!\n");
					ptr5=error_message;
					*ptr5=0x00;
					ptr5++;
					*ptr5=0x05;
					ptr5++;
					*ptr5=0x00;
					ptr5++;
					*ptr5=0x06;
					ptr5++;
					strcpy(ptr5,my_str);
					ptr5=ptr5+len;
					*ptr5=0;
					//printf("%s\n",&error_message[5]);
					//printf("%lu\n",strlen(error_message));
					if((sendbytes=sendto(child_socket,error_message,24,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
				 	  }
					close(child_socket);
					exit(2);
				}
			}
				up_you_go: 	memset(recv_buffer,0,strlen(recv_buffer));
				if((recvbytes= recvfrom(child_socket, recv_buffer, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len))==-1) {
					perror("server_child_recvfrom error");
					
					}
				recv_buffer[recvbytes]='\0';
				printf("received %d bytes \n",recvbytes);
			if(recv_buffer[1]==Data) {
				//printf("It indeed sent a data packet \n");
				uint16_t block_n=(recv_buffer[2]<<8)|(recv_buffer[3]);
					
				if(block_n==expected_block_num) {
					write(fd1,&recv_buffer[4],recvbytes-4);
					memset(send_buffer,0,strlen(send_buffer));
						ptr4=send_buffer;
						*ptr4=0x00;
						ptr4++;
						*ptr4=ACK;
						ptr4++;
						
						if(block_n<=255) {
					
							*ptr4=0x00;
							ptr4++;
							*ptr4=block_n;
							ptr4++;
						}
						else {
							*ptr4=((block_n)&(0xFF00))>>8;
							ptr4++;
							*ptr4=(block_n)&(0x00FF);
							ptr4++;
			 			}
						if((sendbytes=sendto(child_socket,send_buffer,4,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          				 perror("server_child_sendto error");
				   		}
						expected_block_num++;
					if(recvbytes<516) {
						printf("This is the last packet\n");
						printf("Transfer complete\n");
						//printf("Here again\n");
						close(child_socket);
						exit(1);
						
					}

					else if(recvbytes==516) {
						
						goto up_you_go;

					} //else if recvbytes=516 end
				}
			}
						
		   }// strcmp mode octet bracket close	


		if(!strcasecmp(mode,"netascii")) {
			
			char *ptr4;
			char *ptr5;
			unsigned char error_message[520];
			memset(error_message,0,strlen(error_message));
			char my_str[25]="File already exists";
			uint16_t expected_block_num=1;
			int len=strlen(my_str);
			uint16_t block_n;
			int written_bytes;
			ptr4=send_buffer;
			*ptr4=0x00;
			ptr4++;
			*ptr4=ACK;
			ptr4++;
			*ptr4=0x00;
			ptr4++;
			*ptr4=0x00;
			ptr4++;

			if((sendbytes=sendto(child_socket,send_buffer,4,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
				   }

			FILE *my_file;
			my_file=fopen(filename,"wx");
			if(my_file==NULL) {
				printf("[Error]! File already exists !! \n");
					ptr5=error_message;
					*ptr5=0x00;
					ptr5++;
					*ptr5=0x05;
					ptr5++;
					*ptr5=0x00;
					ptr5++;
					*ptr5=0x06;
					ptr5++;
					strcpy(ptr5,my_str);
					ptr5=ptr5+len;
					*ptr5=0;
					//printf("%s\n",&error_message[5]);
					//printf("%lu\n",strlen(error_message));
					if((sendbytes=sendto(child_socket,error_message,24,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          		 perror("server_child_sendto error");
				 	  }
				close(child_socket);
				exit(3);
			}
			
			now_you_see_me: 	memset(recv_buffer,0,strlen(recv_buffer));
				if((recvbytes= recvfrom(child_socket, recv_buffer, MAXBUFLEN-4 , 0,(struct sockaddr *)&their_addr, &addr_len))==-1) {
					perror("server_child_recvfrom error");
					
					}

				recv_buffer[recvbytes]='\0';
				printf("received %d bytes \n",recvbytes-4);
			if(recv_buffer[1]==Data) {
				printf("It indeed sent a data packet \n");
				uint16_t block_n=(recv_buffer[2]<<8)|(recv_buffer[3]);
					ptr4=recv_buffer;
				if(block_n==expected_block_num) {
					written_bytes=write_to_file(my_file,ptr4,recvbytes,recv_buffer);
					memset(send_buffer,0,strlen(send_buffer));
						ptr4=send_buffer;
						*ptr4=0x00;
						ptr4++;
						*ptr4=ACK;
						ptr4++;
						
						if(block_n<=255) {
							//printf("obvio\n");
							*ptr4=0x00;
							ptr4++;
							*ptr4=block_n;
							ptr4++;
						}
						else {
							*ptr4=((block_n)&(0xFF00))>>8;
							ptr4++;
							*ptr4=(block_n)&(0x00FF);
							ptr4++;
			 			}
						if((sendbytes=sendto(child_socket,send_buffer,4,0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
                          				 perror("server_child_sendto error");
				   		}
						expected_block_num++;
					if(recvbytes<516) {
						//printf("This is the last packet\n");
						//printf("Transfer complete\n");
						//printf("Here again\n");
						close(child_socket);
						exit(1);
						
					}

					else if(recvbytes==516) {
						
						goto now_you_see_me;

					} //else if recvbytes=516 end
				}
			}
					
					
					
			
		
						
			





		}  //netascii mode if end 	

		}  //if opcode ==WRQ	

			

			
			
			



    } //fork if closed


  } //while(1) closed



    close(sockfd);
    return 0;
}
