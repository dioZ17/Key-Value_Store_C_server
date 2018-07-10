/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>


#define STORE_DATA 8
#define WORD_L 64
#define BASE_SIZE 64
#define N cmnds[0]
#define I cmnds[2]
#define BLEN 1024

#define SHMSZ 27
#define SHMID 785
int shmid;
//---global variables declared so that they can be used through custom functions---//
//sem_t semf;

void error(char *msg);
char *find_key(char *key);
void put(char *key, char *val);
char *get(char *key);
char * create_store();
void find_ans_next();
void ans_setup(char *word);
void send_answer(int socket, char ans_buff[], int buff_size);
char answer[BLEN];
int curr_ans_cell=0;//answer buffer index
char cmnds[4]={"n\0i\0"};
int exitstat = 0;
char * ans_key; //pointer to answer buffer
char * global_ptr;//pointer to database
int order;//this variable is used for correct answer setup to evade answer like this: /0i<val1>/0i<val2>...
int get_buffclip(char buffer[]);
int bufflen =BLEN;
//void set_bufflen(int argc,char *argv[]);

int main(int argc, char *argv[])
{
	if(argc < 3)error("not enough arguements");

	int pid, count =0;

	order = 0;//0 declares that next command to be written in answer is the 1st command
	//---Create a sample database and initialize pointer---//
	global_ptr=create_store(); 

	//---variable declarations--//
     int sockfd, portno, clilen;
     char buffer[BLEN];
     struct sockaddr_in serv_addr;
     int n;

	//---catch invalid input---//
     if (argc < 2) 
	{
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     	}
	//---open socket---//
     sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

     if (sockfd < 0) 
        error("ERROR opening socket");
	
	//---server_adress struct setup---//
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
	//---bind socket to server adress---//
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) //Binding socket to port
              error("ERROR on binding");
	//---set listening socket---//
     listen(sockfd,0);//listen for connection requests
	
	//---recieve and serve infinite loop---//
	
     printf("Server is listening\n");



     

	int forkcnt; 
	int maxforx=   atoi(argv[2]);
	for(forkcnt=0;forkcnt<maxforx; forkcnt++)
	{
		if((pid = fork() )<0)
		{return 0;}
	else if (pid == 0)
	//---Child Process---//
{	
	printf("Child Process\n");
	while(1){
	struct sockaddr_in cli_addr;
	int newsockfd;
     clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, 		&clilen);//accept connection request
	if (newsockfd < 0) 
          error("ERROR on accept");


	//---shared memory attachment---//
	//char *shm;
//	if ((shm = shmat(shmid,NULL,0)) == (char *)-1){error("shmat");}

	
	printf("server accepted socket\n");
	 bzero(buffer,BLEN);//clearing buffer to store msg
	
	//---recieve socket from client---//
	recv(newsockfd, &buffer, sizeof(buffer)*buffer[0],0);   
	printf("server recieved socket\n");
	//---read socket commands---//
	n = read(newsockfd,buffer,BLEN);
	int i;		
	if (n < 0) error("ERROR reading from socket");//trap error on read

	 //PRINT CONTENTS OF CLIENT BUFFER
	printf("sizeofbuffer: %ld\n", sizeof(buffer));
	for ( i = 0; i < sizeof(buffer); i++ )
  putc( isprint(buffer[i]) ? buffer[i] : '.' , stdout );
	printf("\n");//program gets stuck without this printf
	//---process client's command---//

	char *ptr = buffer;	
		
		//sem_wait(semf);
	printf("essential printf\n");//program gets stuck without this printf
		for ( i = 0; i < sizeof(buffer); i++ )
		{
			//printf("command processing loop %d\n", i);
			if(*ptr == 'p')//process put command//
			{	
			//---Create Pointer to put key---//
				char * val_ptr; 
				val_ptr=ptr;
				ptr++;
			//---Parse through key and point to value---//
				while(*val_ptr!='\0')
				{val_ptr++;}
				val_ptr++;
			//---Put value to database---//
				put(ptr,val_ptr);
	//printf("key: %s\nval: %s\n", ptr, val_ptr);
			//---Set Pointer to next command---//
				ptr = val_ptr;
				while (*ptr!='\0')
				{ptr++;}
				ptr++;
			

			}
		else if(*ptr == 'g')//process get command//
			{
			//---Point to get key---//
				ptr++;
			//---Call get Function---//
				get(ptr);
				if(exitstat>0)
				{
				send_answer(newsockfd,answer,get_buffclip(answer));
				i=sizeof(buffer);
				}				
			//---Set Pointer to next Command---//
				while (*ptr!='\0')
				{ptr++;}
				ptr++;
					
		
			}else //---if no get nor put: do nothing---//
			{
				//printf("... ");
				//i=sizeof(buffer);
			}

			


		}//printf("\n");
		//sem_post(semf);
	//-----------print data----------------//
	for(i=0; i<(STORE_DATA); i++)
	{
		printf("DATA #%d %s\n",i+1,global_ptr+(i*WORD_L));
	}
	//---Print Answer---//
	for ( i = 0; i < sizeof(answer); i++ )
  putc( isprint(answer[i]) ? answer[i] : '.' , stdout );

	printf("Bufflcip: %d\n", get_buffclip(answer));
	//---Send Answer---//
    if(global_ptr != '\0')
	 n = write(newsockfd,answer,get_buffclip(answer));
     if (n < 0) error("ERROR writing to socket");

	//send_answer(newsockfd, answer, 256);
	curr_ans_cell=0;
	bzero(answer,BLEN);
	order = 0;
	exitstat = 0;
	close(newsockfd);
	printf("Child finished execution\n");
	}//end of infinte loop
	return 0;}//end of child Process
	else{//---parent process---//
	//close(newsockfd);
	//wait(NULL);		
		}
	
	printf("Forking %d\n", forkcnt);
	
	}//end of forking loop   
	//printf("infinity ended, closing socket?\n");
	close(sockfd);  
	return 0; 
}


//=========FUNCTIONS================//

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
//	printf("Buffclip Loop: %d : nccnt: %d\n", i, nncnt);
		
	}
	return buffclip;
}


void error(char *msg)
{
    perror(msg);
    exit(1);
}

void put(char *key, char *val)
{
	//printf("executing PUT\n");
	char * ptr = find_key(key);
	ptr+=WORD_L;
	bzero(ptr, WORD_L);
	strcpy(ptr, val);
	
	
}
char *get(char *key)
{
	
	char * getkey;
	getkey = &N;
	if(strcmp(find_key(key),getkey)==0)
	{
		//printf("KEY NOT FOUND!!!\n");
		ans_setup(getkey);
		return getkey;
	}	
	//getkey=&keyfound;//Predict that requested key will be found in datab
	
	getkey = &I;
	ans_setup(getkey);//write i to answer
	//if(strcmp(ans_key, getkey)==0)
	//return ans_key;
	
	getkey = find_key(key)+WORD_L;	
	ans_setup(getkey);
	
	
	return find_key(key)+WORD_L;
	
}
//---This function parses through the database to match they key and returns a pointer to the answer buffer---//
char * find_key(char *key)
{
	char *ptr;
	ptr = global_ptr;//ptr points to 1st cell of database
	int i;
	for(i=0; i<(STORE_DATA); i++)//Parse through all database values
	{
		if(strcmp(key, ptr)==0)
			{
				//printf("Found Key as: %s\n", ptr);
				return ptr;//return pointer to requested key
			}
		else//increment pointer to next data value
			ptr+=WORD_L;
	}
	//---this part of code executes only if the requested key was not found in the database 
	ptr = &N;
	//printf("No such key found in database\nptr %s\n",ptr);
	return ptr;
}
//---This function sets the global variable curr_ans_cell to the next empty cell of the answer---//
void find_ans_next()
{
	char *empty_ptr;
	empty_ptr=&answer[curr_ans_cell];
	while(*empty_ptr!='\0')
	{
		//printf("search to infinity\n");
		curr_ans_cell++;
		empty_ptr++;
	}
	
}
//---This function copies the word arguement to the answer buffer---//
void ans_setup(char * word)
{	
	
	char *ptr;
	
	find_ans_next();//set answer index to the next empty cell
	//---Increase answer index to point to second null char---//
	//---note: not best way to do it but...code developement..---//	
	//if((order>0) && (strcmp(word, &keyfound))==0)
	/*	
	if(((order>0) && (strcmp(word, &I) || strcmp(word, &N)) )==0)	
	{
	 curr_ans_cell+=1;
	}else{order=1;}
	*/
	if((order>0) && (strcmp(word, &I))==0)	
	{
	 curr_ans_cell+=1;
	}else if((order>0) && (strcmp(word, &N))==0)
	{
		curr_ans_cell+=1;
		exitstat = 1;
	}
	else{order=1;}
	
	ptr = &answer[curr_ans_cell];
	
	strcpy(ptr,word);
	
		//...
		//if((strcmp(word, &keyfound))==0)
	/*	if((strcmp(word, &I))==0)
		{
			return 0;
		}else{return -1;}*/
}

void send_answer(int socket, char ans_buff[], int buff_size)
{
	char *ptr;
	int i;
	int exit = -1;
	ptr = &ans_buff[0];

	for(i=0; i<buff_size; i++)//parse through answer buffer
	{
		if(*ptr=='\0')//after 2nd NULL prepare to exit
		{	exit++;
			if(exit>0)//enter if 2 consecutive NULL chars
			{
				if(i<3)//return if ans=n // trap later decreasing ptr EOF
				{
					if (write(socket,ans_buff,1) < 0) error("ERROR writing to socket");
					curr_ans_cell=0;
					bzero(ans_buff,1);
				}
				//by now our ptr points to the second NULL char of ans
				//int tailcut=buff_size-3;//set to cut
				if(strcmp(ptr,"\0n\0"))//Discern end of <VAL> or not found
				{i-=2;}else{i-=1;}	//choose last cell to send address
					if (write(socket,ans_buff,i) < 0) error("ERROR writing to socket");
					curr_ans_cell=0;
					bzero(ans_buff,BLEN);

			}
		//reset exit status
		exit = -1;
		}
	}
    
}
//ABOUT KEY-VALUE DATABASE
//To create the database we will use a custom array made from scratch with 
//fixed word bytesize and fixed number of double word slots or <KEY><VALUE>
//To initialize sample keys with values we fill a temporary buffer
//This buffer does not follow the set rules so we fill the database using a
//loop with wordsize incrementing pointer
//Lastly we return a pointer to the first cell of our database 
//Comments: As this is not a project about databases the database code is not
//protected by -example- keys/values with larger size than <word_length> 
//EDIT: using 

char * create_store()
{
	//declare database variables//
	const int base_bytes = WORD_L*2*BASE_SIZE;
	//int shmid;
	key_t key = SHMID;
	char *shm;
	
	if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0)
	error("shmget");
	if((shm = shmat(shmid,NULL,0))==(char *) -1) 
	error("shmat");

	//allocate and clear database//
	//char base[base_bytes];
	//bzero(base, base_bytes);
	//fill temporary buffer
	char buffer[256] = {"class\0lock\0race\0midget\0level\09000\0alignment\0LE\0"};
	char *bsptr;
	bsptr = shm;
	char *bfptr = buffer;
	bzero(shm, base_bytes);
	//fill base//
	int i;
	strcpy(bsptr,bfptr);
	for(i=0; i<(STORE_DATA-1); i++)
	{
		printf("DATA #%d %s\n",i+1,bsptr);
		bsptr+=WORD_L;
		while(*bfptr!='\0')
		{bfptr++;}
		bfptr++; 
		strcpy(bsptr,bfptr);
	}
	printf("DATA #%d %s\n",i+1,bsptr);
	bsptr = shm;
	return bsptr;
	return bfptr;
}

