#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFF_SIZE 64

int cur_cell_num = 0;//Global Variable of current cell of buffer
int bufflen=0;	

void error(char *msg);
int findBuffNext(char buffer[], int size);
void print_buffer(char buffer[], int size);
int get_buffclip(char buffer[]);

//Need a function that traps buffer overflow//
//MAKE AS: count total arguement bytes by strlen*argc. after: compare ..
//manage buffer overflow


//-=-=-=-=-=-=-==-=-=-==-=-=-=-===-=-=-=-=-=-=-=-=-==-=-=-///
			//MAIN///


int main(int argc, char *argv[])
{
	//---Buffer Length Management---//
	//Bufflen var will be larger than needed socket buffer :(
	int i;
	
	for(i=0; i<argc; i++)
	{
		bufflen+=strlen(argv[i]);//count chars in args		
	}
	
	char buffer[bufflen];
	int sockfd, portno, n;

	struct sockaddr_in serv_addr;//create structs
	struct hostent *server;

	
	//int buf_size = sizeof(buffer)/sizeof(buffer[0]);	
	

	if (argc < 3){	//trap arguements < 3//
	fprintf(stderr, "usage %s hostname port\n", argv[0]);
	exit(1);
		}
	//---set port number---//
	portno = atoi(argv[2]);			//set set Port Number as 2nd arguement 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//Create the default Socket
	if (sockfd < 0)				//trap socket creation Error
		error("ERROR opening socket");

	//---set server ip as 1st arguement---//
	server = gethostbyname(argv[1]);	
	if(server == NULL) {	//trap no 1st arguement??
		fprintf(stderr, "ERROR, no suck host\n");
		exit(1);
			}

bzero((char*)&serv_addr,sizeof(serv_addr)); 	//erease server adress struct
serv_addr.sin_family = AF_INET;	//Start setting struct values. Set Family(v4-v6)
bcopy((char *)server->h_addr,
	(char*)&serv_addr.sin_addr.s_addr,
	server->h_length);
//printf("Checkpoint: Setting Port Number \n");
serv_addr.sin_port = htons(portno);//set port number
printf("Checkpoint: Initializing Connection\n");			


	
			//---Connect---//
if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	error("ERROR connecting");//trap connection to server fail
printf("Checkpoint: Connection Complete\n");

//Instruction Management II//
//In this section of code we will manage the input and store it in a buffer that will later be sent to the server through the socket. 
//The order will be (g<KEY>\0 | p<KEY>\0<VAL>\0)*

bzero(buffer, bufflen);//clear buffer


int order=0;//initial order space: this will change to 1 after the first put/get
i=3;//loop counter. val=3 so it can jump over ip and port 
char * buf_ptr, argv_ptr; //pointers to buffer and arguements
buf_ptr = buffer;//initialize buffer pointer to 1st element of buffer

while(i<argc)//parse through rest arguements
	{
		if(strcmp(argv[i], "get")==0)//get order manage
		{
	//inc the global var to jump over the \0 char of the last command. this will be a +0 in case it's the 1st command
		cur_cell_num+=order;
buf_ptr =&buffer[findBuffNext(buffer,bufflen)];//find next terminal char
		*buf_ptr=(char)0x67;		//write get opcode in buffer
buf_ptr=&buffer[findBuffNext(buffer, bufflen)];//move ptr to slot next to g_opcode
		i++;//inc i to get get key
		strcpy(buf_ptr, argv[i]);//move key to buffer		
		buf_ptr=&buffer[findBuffNext(buffer,bufflen)];//update buf pointer to point at key's terminal char in buffer

		}else if(strcmp(argv[i], "put")==0)
		{
		cur_cell_num+=order;//<<same as above>>
//buf_ptr = buffer;//initialize buffer pointer to 1st element of buffer
buf_ptr=&buffer[findBuffNext(buffer, bufflen)];//find terminal char
		*buf_ptr=(char)0x70;		//write put opcode in buffer
buf_ptr=&buffer[findBuffNext(buffer, bufflen)];//point to slot next to p_opcode
	
		i++;//inc i to get put key
		strcpy(buf_ptr, argv[i]);//move key 2 buffer directly behind put
		argv_ptr++;//point to next arguement(value)
buf_ptr=&buffer[findBuffNext(buffer, bufflen)];//point to key's terminal char
		i++;//inc i to get put value
		//To pass a \0 between <KEY> and <VAL>
		cur_cell_num++;//index pointing to -soon2be- 1st letter of <VAL>
		strcpy(buf_ptr+1, argv[i]);//write p_val to buffer NEXT 2 k_term_char
		buf_ptr=&buffer[findBuffNext(buffer,bufflen)];//update buf pointer to point at p_value's terminal character
		}
		else{printf("invalid command: %s\n exiting\n", argv[i]);exit(1);}//catch invalid command
	order=1;//after 1st loop order is set to one so we can write the next opcode correctly (can be bypassed with another findBuffNext call)

	i++;//inc i for next opcode or EOF
	}
	//---clip socket tail---//

	printf("sending socket...\n");
	n = send(sockfd, &buffer,get_buffclip(buffer),0);// sizeof(buffer)*buffer[0], 0);
	if(n<0)
	{error("ERROR sending socket");}

	printf("socket has been sent\n");

	bzero(buffer,bufflen);//erease buffer memory
	n = read(sockfd,buffer,bufflen);//read buffer copy we from socket
	if(n<0)
		error("ERROR reading from socket");


	//for ( i = 0; i < get_buffclip(buffer); i++ )
	//  putc( isprint(buffer[i]) ? buffer[i] : '.' , stdout );
	//printf("\n");
	

	//---Translate and Print Buffer---//
	char *ptr = &buffer[0];
	ptr++;
	printf("%s\n", ptr);
			
	for(i=1; i<get_buffclip(buffer);i++)
	{
	

		
		if(buffer[i]=='\0')
		{
			ptr=&buffer[i];
			ptr+=2;
			printf("%s\n",ptr);
		}
			
	}


	close(sockfd);
	return 0;
	}



//=-=-=-=-=-=-=-=-=-=-=-=FUNCTIONS=-===-=-=-=-=-=-=-=-=-//
void error(char *msg)
{
	perror(msg);
	exit(1);
}
//function that returns the index with the next \0 in buffer. It is updated each time it is being called using global variable cur_cell_num
//Note that if the current cell is \0 the function will return the index to this
int findBuffNext(char buffer[], int size)//pointer to char return
{
	//printf("size of buffer: %ld\n", sizeof(buffer)/sizeof(buffer[0]));
	char *ptr = &buffer[cur_cell_num];
	int i;
	for(i=cur_cell_num; i<size; i++)
		{
		//printf("BufferValue: %c\n",*ptr);
			if(strcmp(ptr,"\0")==0)
				{
				cur_cell_num=i;
	//printf("Cell Num Value is now: %d\n", cur_cell_num);
				return i;
				}ptr++;
		}
}

void print_buffer(char buffer[], int size) //function that prints the buffer
{
	int i;
//printf("Printing Buffer: will stop on 1st \\0\n%s\n",buffer);
	printf("Buffer:\n");
	for(i=0; i<size; i++)
	{
		printf("%c",buffer[i]);
	}
	printf("\n");
}


//This function returns an integer as the minimal size of the buffer. 
//It clips aways the extra Null characters at the end of the buffer
int get_buffclip(char buffer[])
{
	
	int nncnt=0; //NullNullCounter
	int i;
	int buffclip=0;
	for(i=0;i<bufflen;i++)
	{
		
		if(buffer[i]=='\0')
		{
			nncnt++;
		}
		else nncnt=0;
		
		if(nncnt>=2)
		{
		buffclip=i;
		i=bufflen;
		}
	//printf("Buffclip Loop: %d : nccnt: %d\n", i, nncnt);
		
	}
	return buffclip;
}
















