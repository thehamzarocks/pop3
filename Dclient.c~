#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

int main()
{
 struct sockaddr_in sock_var;
 int cfd=socket(AF_INET,SOCK_STREAM,0);
 char str_clnt[2000],str_ser[2000];
 
 sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
 sock_var.sin_port=3000;
 sock_var.sin_family=AF_INET;
 
 if(connect(cfd,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
 {
  printf("Connected to server %d\n",cfd);
  //printf("nEnter Srting to send");
  //scanf("%s",str_clnt);
  //write(clientFileDiscriptor,str_clnt,20);
  read(cfd,str_ser,2000);
  printf("%s",str_ser);

  while(1) {
	  //scanf("%s",str_clnt);
	  gets(str_clnt);
	  write(cfd, str_clnt, 2000);
	  read(cfd, str_ser, 2000);
	  printf("%s\n",str_ser);
  }
  close(cfd);
 }
 else{
  printf("socket creation failed");
 }
 return 0;
}
