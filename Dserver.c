#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

/*void *ServerEcho(void *args)
{
 int clientFileDiscriptor=(int)args;
 char str[20];

 read(clientFileDiscriptor,str,20);
 printf("nreading from client:%s",str);
 write(clientFileDiscriptor,str,20);
 printf("nechoing back to client");
 close(clientFileDiscriptor);
}*/

int state;


int UserNameInput(int cfd, char str[2000]) {
	int i;
	char username[2000];

	//read(cfd, str, 2000);

	if(state != 1) {
		sprintf(str, "Not in the right state");
		write(cfd, str, 2000);
		return -1;
	}

	int j = 0;
	for(i=5;str[i]!='\0';i++) {
		username[j++] = str[i];
	}
	username[j] = '\0';

	char path[2000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");
	
	if(fp == NULL) {
		sprintf(str, "Unknown Username\n");
		write(cfd, str, 2000);
		return -1;
	}


	sprintf(str, "+OK Hello %s, now send PASS",username);
	state = 2;

	write(cfd, str, 2000);
	return 1;
	
}

int PasswordInput(int cfd, char str[2000]) {
	if(state != 2) {
		sprintf(str, "Not in the right state");
		write(cfd, str, 2000);
		return -1;
	}

	int j = 0;
	int i;
	char pass[2000];
	for(i=5;str[i]!='\0';i++) {
		pass[j++] = str[i];
	}
	pass[j] = '\0';

	sprintf(str, "+OK, Welcome and proceed");
	state = 3;
	write(cfd, str, 2000);
	return 1;
}

void parse(int cfd, char str[2000]) {
	if(strncmp("USER", str, 4) == 0) {
		UserNameInput(cfd, str);
	}

	else if(strncmp("PASS", str, 4)==0) {
		PasswordInput(cfd, str);
	}

	else if(strncmp("STAT", str, 4)==0) {
	}

}

void *ServerPOP3(void *args) {
	int cfd = (int)args;
	char str[2000];
	int i;

	strcpy(str, "+OK, connected\n");
	write(cfd, str, 2000);
	
	/*if(UserNameInput(cfd, str) == 1) { //accepted user
		state = 2; 
	}*/

	state = 1;

	while(1) {
		read(cfd, str, 2000);
		parse(cfd, str);
	}



	



	close(cfd);
}



int main()
{
 struct sockaddr_in sock_var;
 int serverFileDiscriptor=socket(AF_INET,SOCK_STREAM,0);
 int clientFileDiscriptor;
 int i;
 pthread_t t[20];
 
 sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
 sock_var.sin_port=3000;
 sock_var.sin_family=AF_INET;
 if(bind(serverFileDiscriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
 {
  printf("nsocket has been created");
  listen(serverFileDiscriptor,0); 
  while(1)        //loop infinity
  {
   for(i=0;i<20;i++)      //can support 20 clients at a time
   {
    clientFileDiscriptor=accept(serverFileDiscriptor,NULL,NULL);
    printf("nConnected to client %dn",clientFileDiscriptor);
    pthread_create(&t,NULL,ServerPOP3,(void *)clientFileDiscriptor);
   }
  }
  close(serverFileDiscriptor);
 }
 else{
  printf("nsocket creation failed");
 }
 return 0;
}
