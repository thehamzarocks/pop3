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
char marked[100][2000];
int nummarked;

int lock = 1;


int UserNameInput(int cfd, char str[2000], char username[2000]) {
	int i;

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

int PasswordInput(int cfd, char str[2000], char username[2000]) {
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

	char path[2000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");

	char actualpassword[2000];
	fgets(actualpassword, 2000, fp);
	if(strncmp(actualpassword, pass,strlen(pass))!=0) {
		sprintf(str, "Wrong password");
		write(cfd, str, 2000);
		return -1;
	}


	sprintf(str, "+OK, Welcome and proceed");
	state = 3;
	write(cfd, str, 2000);
	return 1;
}

int GetMessageCounts(int cfd, char username[2000]) {
	int i;
	char path[2000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");

	int nn = -1; //since one line is for the password
	char buf[20000];
   	while(fgets(buf, 20000, fp) != NULL) {
		nn++;
	}

	int mm = 0;
	fp = fopen(path, "r");
	fgets(buf, 20000, fp); //ignore the first line as it stores the password

	while(fgets(buf, 20000, fp) != NULL) { //small problem here
		mm += strlen(buf);
	}

	sprintf(buf, "+OK, %d %d",nn,mm);
	write(cfd, buf, 2000);
	return 1;
}

int ListMessages(int cfd, char str[20000], char username[2000]) {
	int i;
	char path[2000];
	char buf[20000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");

	fgets(buf, 20000, fp);

	char msglist[20000];
	sprintf(msglist, "+OK");

	char msgid[2000];

	while(fgets(buf, 20000, fp) != NULL) {
		int j = 0;
		for(i=0;buf[i]!='.';i++) {
			msgid[j++] = buf[i];
		}
		msgid[j] = '\0';
		sprintf(msglist, "%s\n%s",msglist,msgid);
	}

	sprintf(msglist, "%s\n.",msglist);
	write(cfd, msglist, 2000);
	return 1;
}

int RetrieveMessage(int cfd, char str[2000], char username[2000]) {
	int i;
	char path[2000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");

	char msgid[2000];
	int j = 0;
	for(i=5;str[i]!='\0';i++) { //retrieve the message id from the string
		msgid[j++] = str[i];
	}
	msgid[j] = '\0';

	char buf[2000];
	fgets(buf, 2000, fp);

	char msg[2000];
	int found = 0;
	while(fgets(buf,2000,fp) != NULL) {
		if(strncmp(msgid, buf, strlen(msgid))==0) { //matching message
			sprintf(msg, "+OK Message Found\n%s",buf);
			found = 1;
			break;
			//write(cfd,msg,20000);
			//return 1;
		}
	}

	if(found == 1) {
		write(cfd, msg, 2000);
		found = 0;
		return 1;
	}
	sprintf(buf, "-ERR Message Not Found");
	write(cfd, buf, 2000);
	return -1;
}

int MarkForDeletion(int cfd, char str[2000], char username[2000]) {
	int i;

	char path[2000];
	sprintf(path, "Users/%s",username);
	FILE *fp = fopen(path, "r");
	FILE *fout = fopen("Users/temp", "w");
	
	char msgid[2000];
	int j = 0;
	for(i=5; str[i]!='\0'; i++) {
		msgid[j++] = str[i];
	}
	msgid[j] = '\0';

	char buf[2000];
	fgets(buf, 2000, fp);
	fprintf(fout, "%s",buf);
	int found = 0;

	char msg[2000];
	while(fgets(buf, 2000, fp) != NULL) {
		if(strncmp(buf, msgid, strlen(msgid))==0) { //okay to compare only up to length of msgid as messages are in ascending order of ids
			found = 1;
			sprintf(msg, "+OK Message Deleted");
		}
		else { //copy everything but the matched message
			fprintf(fout, "%s",buf);
		}
	}


	fclose(fout);
	remove(path);
	rename("Users/temp",path);
	if(found == 1) {
		write(cfd, msg, 2000);
		return 1;
	}
	else {
		sprintf(msg, "-ERR Message Not Found");
		write(cfd, msg, 2000);
		return 1;
	}
}

	

void parse(int cfd, char str[2000], char username[2000]) {
	if(strncmp("USER", str, 4) == 0) {
		UserNameInput(cfd, str, username);
	}

	else if(strncmp("PASS", str, 4)==0) {
		PasswordInput(cfd, str, username);
	}

	else if(strncmp("STAT", str, 4)==0) {
		GetMessageCounts(cfd, username);
	}
	else if(strncmp("LIST", str, 4)==0) {
		ListMessages(cfd,str, username);
	}
	else if(strncmp("RETR", str, 4)==0) {
		RetrieveMessage(cfd, str, username);
	}
	else if(strncmp("DELE", str, 4)==0) {
		MarkForDeletion(cfd, str, username);
	}


}

void *ServerPOP3(void *args) {
	int cfd = (int)args;
	char str[2000];
	char username[2000];
	int i;

	if(lock == 1) {
		lock = 0;
		strcpy(str, "+OK, connected\n");
		write(cfd, str, 2000);
	}
	else {
		strcpy(str, "-ERR Unable to connect\n");
		write(cfd, str, 2000);
		close(cfd);
		return;
	}
	
	/*if(UserNameInput(cfd, str) == 1) { //accepted user
		state = 2; 
	}*/

	state = 1;
	nummarked = 0;

	while(1) {
		read(cfd, str, 2000);
		parse(cfd, str, username);
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
