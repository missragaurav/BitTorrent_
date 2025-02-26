#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include <termios.h>
#include<signal.h>
#include<iostream>
#include<cstring>
#include<sstream>
#include <openssl/sha.h>
#include<sys/wait.h> 
using namespace std;
struct us
{
	string u,sh,s,na;
}mm;
int sockfd = 0; 
string opt;
char user12[50];
struct sockaddr_in serv_addr,serv_addr1;
int listenfd = 0, connfd[100],ii=0;
pthread_t thread[100],t1;
void sigpipe_handler(int unused)
{
	cout<<"\nclose1\n";
}
void handle(int a)
{
	cout<<"\nclose\n";
	close(sockfd);
}
void *client(void *arg)
{
	int fd=open("tracker.txt",O_RDWR,0666);
	char c;
	bool b=false;
	string u="",p="",user=mm.u,name=mm.na,size=mm.s;
	while(read(fd,&c,1)>0)
	{
		if(c==':')
		b=true;
		else if(c=='\n')
		{
			if(u==user)
			break;
			u="";
			p="";
			b=false;
		}	
		else if(b==false)
		u+=c;
		else if(b==true)
		p+=c;
	}
	close(fd);
	int argc=3,sockfd1;
	char *argv[]={"h","127.0.0.1"};
	argv[2]=(char *)malloc(p.length());
	strcpy(argv[2],p.c_str());
	
	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> \n",argv[0]);
		pthread_exit(NULL);
	} 
	if((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		pthread_exit(NULL);
	} 
	memset(&serv_addr1, '0', sizeof(serv_addr1)); 
	serv_addr1.sin_family = AF_INET;
	int port;
	sscanf(argv[2], "%d", &port);
	serv_addr1.sin_port = htons(port); 
	printf("Server address used is: %s\n", argv[1]);
	if(inet_pton(AF_INET, argv[1], &serv_addr1.sin_addr)<=0)
	{
		printf("\ninet_pton error occured\n");
		pthread_exit(NULL);
	} 
	if( connect(sockfd1, (struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		pthread_exit(NULL);
	} 
	char t[50]={0};
	strcpy(t,name.c_str());
	cout<<t<<endl;
	send(sockfd1,t,50,0);
	int size1=0;
	char *f;
	recv(sockfd1, &size1, sizeof(int),0);
	f=(char *)malloc(size1);
	recv(sockfd1,f,size1,0);
	int fd1=open(t,O_RDWR|O_CREAT,0666);
	write(fd1,f,size1);
	close(fd1);
	close(sockfd1);
	pthread_exit(NULL);
}
void *sendd(void *arg)
{
	int connfd=*((int*)(&arg));
	char t[50]={0};		
	recv(connfd,t,50,0);
	struct stat file;
	int size;
	stat(t,&file);
	size=file.st_size;
	send(connfd, &size, sizeof(int),0);
	int fd=open(t,O_RDWR,0666);
	sendfile(connfd,fd,NULL,size);
	close(connfd);
	close(fd);
}
void *serverC(void *arg)
{
	pthread_t t3[100];
	int fd=open("tracker.txt",O_RDWR,0666);
	char c;
	bool b=false;
	string u="",p="",user(user12);
	while(read(fd,&c,1)>0)
	{
		if(c==':')
		b=true;
		else if(c=='\n')
		{
			if(u==user)
			break;
			u="";
			p="";
			b=false;
		}	
		else if(b==false)
		u+=c;
		else if(b==true)
		p+=c;
	}
	close(fd);
	cout<<p<<endl;
	int port=stoi(p);
	struct sockaddr_in serv_addr; 
   	listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//int port;
	//sscanf(argv[1], "%d", &port);
	serv_addr.sin_port = htons(port); 
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
	listen(listenfd, 10); 
	//fflush(stdin);
	int i=0,ci=0;
	while(1)
	{
		connfd[ci] = accept(listenfd, (struct sockaddr*)NULL, NULL);
		pthread_create(&t3[ci],NULL,&sendd,(void *)connfd[ci]); 
		ci++;
	}
	
}
int main(int argc, char *argv[])
{
	pthread_t t2;
	string uu="";
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGINT,handle);
	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> \n",argv[0]);
		return 1;
	} 
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	} 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET;
	int port;
	sscanf(argv[2], "%d", &port);
	serv_addr.sin_port = htons(port); 
	printf("Server address used is: %s\n", argv[1]);
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\ninet_pton error occured\n");
		return 1;
	} 
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	} 
	while(1)
	{
		//fflush(stdin);
		//signal(SIGINT,handle);
		getline(cin,opt);
		string word="";
		stringstream s(opt);
		s >> word;
		if(word=="c_u")
		{
			
			send(sockfd,"create_user",11,0);
			string user="",pass="";
			s >> user;
			s >> pass;
			char user1[1024]={0},pass1[1024]={0};
			strcpy(user1,user.c_str());
			strcpy(pass1,pass.c_str());
			send(sockfd,user1,1024,0);
			send(sockfd,pass1,1024,0);
			char aut[2]={0};
			recv(sockfd,aut,1,0);
		}
		else if(word=="login")
		{
			send(sockfd,"login",11,0);
			string user="",pass="";
			s >> user;
			s >> pass;
			char user1[1024]={0},pass1[1024]={0};
			strcpy(user1,user.c_str());
			strcpy(pass1,pass.c_str());
			send(sockfd,user1,1024,0);
			send(sockfd,pass1,1024,0);
			char aut[2]={0};
			recv(sockfd,aut,1,0);
			string aut1(aut);
			pthread_t yy;
			if(aut1=="1")
			{
				cout<<"Login Suc\n";
				uu=user;
				strcpy(user12,uu.c_str());
				pthread_create(&yy,NULL,&serverC,NULL);
			}
			else
			{
				cout<<"Fail\n";
				uu="";
			}
		}	
		else if(word=="c_g")
		{
			string g="";
			s >> g;
			send(sockfd,"create_group",12,0);
			if(uu!="")
			{
				char p[100]={0};
				string k=g+":"+uu;
				strcpy(p,k.c_str());
				send(sockfd,p,100,0);
				recv(sockfd,p,2,0);
				memset(&p, '0', sizeof(p)); 
			}			
		}
		else if(word=="l_g")
		{
			send(sockfd,"list_group",10,0);
			int size=0;
			char *f;
			recv(sockfd, &size, sizeof(int),0);
			if(size!=0)
			{
				f=(char *)malloc(size);
				recv(sockfd,f,size-1,0);
				cout<<f<<endl;
			}
		}
		else if(word=="j_g")
		{
			send(sockfd,"join_group",10,0);
			string g="";
			s >> g;
			char p[100]={0};
			strcpy(p,g.c_str());
			send(sockfd,p,100,0);	
		}
		else if(word=="l_r")
		{
			send(sockfd,"list_request",12,0);
			int size=0;
			char *f;
			string ff;
			s >> ff;
			recv(sockfd, &size, sizeof(int),0);
			if(size!=0)
			{
				string g="",u="";
				int b=0;
				char c;
				f=(char *)malloc(size);			
				recv(sockfd,f,size,0);
				cout<<f<<endl;
			}
		}
		else if(word=="a_r")
		{
			send(sockfd,"accept_request",14,0);
			string g="",u="";
			s >> g;
			s >> u;
			char p[100]={0};
			g=g+":"+u;
			strcpy(p,g.c_str());
			send(sockfd,p,g.length(),0);
			sleep(2);
		}
		else if(word=="upload_file")
		{
			send(sockfd,"upload_file",14,0);
			string g,f,kk="";
			s >> f;
			s >> g;
			char p[100]={0},tt[3];
			unsigned char *buff;
			strcpy(p,f.c_str());
			struct stat file;
			int size,size1;
			stat(p,&file);
			size=file.st_size;
			size1=size;
			if(size>20)
			{
				size=20;
				
			}
			buff=(unsigned char *)malloc(size);
			int fd=open(p,O_RDWR,0666);
			read(fd,buff,size);
			close(fd);
			const unsigned char *t=buff;
			char unsigned result[SHA_DIGEST_LENGTH];
			SHA1(t,size, result);
   			for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
   			{
				sprintf(tt,"%02x",result[i]);
				string k(tt);
				kk+=k;
			}		
			f=g+":"+f+":"+to_string(size1)+":"+kk;
			char pp[100]={0};
			strcpy(pp,f.c_str());
			cout<<pp<<endl;
			send(sockfd,pp,f.length(),0);
			sleep(5);
		}
		else if(word=="download_file")
		{
			send(sockfd,"download_file",14,0);
			string g,f,p,gg="",uu="";
			bool b=true,m=true;
			char c;
			s >> g;
			s >> f;
			f=g+":"+f;
			char pp[100]={0},ppp[4]={0};
			strcpy(pp,f.c_str());
			send(sockfd,pp,f.length(),0);
			recv(sockfd,ppp,3,0);
			string re(ppp);
			if(re=="0")
			{
				cout<<"Wait till download :P\n";
				char t[100]={0};				
				recv(sockfd,t,100,0);
				int num=0;
				string user="",name="",size="",sha="";
				for(int i=0;t[i]!='\0';i++)
				{
					if(t[i]==':')
					num++;
					else if(num==0)
					{
						user+=t[i];
					}
					else if(num==1)
					{
						name+=t[i];
					}
					else if(num==2)
					{
						size+=t[i];
					}
					else if(num==3)
					{
						sha+=t[i];
					}
				}	
				cout<<user<<" "<<name<<endl;
				mm.u=user;
				mm.s=size;
				mm.sh=sha;
				mm.na=name;		
				pthread_create(&t1,NULL,&client,NULL);
				pthread_join(t1,NULL);
				//sleep(2);
				fflush(stdin);
			}
			
		}
		else if(word=="logout")
		{
			send(sockfd,"logout",6,0);
			uu="";	
			exit(0);		
		}				
	}
close(sockfd);
return 0;
}