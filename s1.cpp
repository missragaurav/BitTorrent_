#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include<iostream>
#include<cstring>
#include<signal.h>
#include<map>
using namespace std;
int listenfd = 0, connfd[100];
pthread_t thread[100];
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER,l1=PTHREAD_MUTEX_INITIALIZER,l2=PTHREAD_MUTEX_INITIALIZER;
map<string,string> user_pass;
map<string,string> group;
map<string,string> c_id;
int ii=0,ci=0;
bool loginC[100];
void sigpipe_handler(int unused)
{
	cout<<"\nclose1\n";
}
void handle(int a)
{
	cout<<"\nclose\n";
	for(int i=0;i<10;i++)
	close(connfd[i]);
	pthread_mutex_destroy(&lock);
	exit(0);
}
void set_User()
{
	signal(SIGINT,handle);
	user_pass.clear();
	char c;
	bool b=true;
	string user="",pass="";
	//pthread_mutex_lock(&lock);
	int fd=open("username.txt",O_RDWR|O_CREAT,0666);
	while((read(fd,&c,1))!=0)
	{
		if(c==':')
		{
			b=false;
		}
		else if(c=='\n')
		{
			cout<<user<<" "<<pass<<endl;
			user_pass.insert(make_pair(user,pass));
			b=true;
			user="";
			pass="";
		}
		else if(b==true)
		{
			user+=c;
		}
		else if(b==false)
		{
			pass+=c;
		}
	}
	fd=open("group.txt",O_RDWR|O_CREAT,0666);
	b=true;
	user="";
	pass="";
	while((read(fd,&c,1))!=0)
	{
		if(c==':')
		{
			b=false;
		}
		else if(c=='\n')
		{
			group.insert(make_pair(user,pass));
			b=true;
			user="";
			pass="";
		}
		else if(b==true)
		{
			user+=c;
		}
		else if(b==false)
		{
			pass+=c;
		}
	}
	//pthread_mutex_unlock(&lock); 
}
void createId(void *arg)
{
	signal(SIGINT,handle);
 	char user[1024]={0},pass[1024]={0};
	int cid =*((int*)(&arg));
	recv(cid,user,1024,0);
	recv(cid,pass,1024,0);
	string u(user),p(pass);
	pthread_mutex_lock(&lock); 
	user_pass.insert(make_pair(u,p));
	int fd=open("username.txt",O_RDWR|O_CREAT|O_APPEND,0666);
	write(fd,user,u.length());
	write(fd,":",1);
	pass[p.length()]='\n';
	pass[p.length()+1]='\0';
	write(fd,pass,p.length()+1);
	pthread_mutex_unlock(&lock); 
	send(cid,"1",1,0);
	close(fd);	
}
void login(void *arg)
{
	signal(SIGINT,handle);
 	char user[1024]={0},pass[1024]={0};
	int cid =*((int*)(&arg));
	fflush(stdout);
	recv(cid,user,1024,0);
	recv(cid,pass,1024,0);
	string u(user),p(pass);
	auto it =user_pass.find(u);
	if(it!=user_pass.end())
	{
		if(it->second==p)
		{
			loginC[cid]=true;
			c_id.insert(make_pair(to_string(cid),u));
			cout<<"Match\n";
			send(cid,"1",1,0);
		}
		else
		{
			loginC[cid]=false;
			cout<<"Unmatch\n";
			send(cid,"0",1,0);
		}
	}
	else
	{
		loginC[cid]=false;
		cout<<"Not Found\n";
		send(cid,"0",1,0);
	} 	
}
void *menu(void *arg)
{
	int cc =*((int*)(&arg));
	while(1)
	{
		char opt[100]={0};
		sleep(1);
		recv(cc,opt,100,0);
		string opt1(opt);
		if(opt1=="create_user")
		{	
			cout<<"C_R\n";	
			createId((void *)cc);
		}
		else if(opt1=="login")
		{
			login((void *)cc);
		}
		else if(opt1=="create_group")
		{
			if(loginC[cc]==true)
			{
				pthread_mutex_lock(&l1); 
				char p[100]={0};
				sleep(1);
				recv(cc,p,100,0);
				string s(p);
				p[s.length()]='\n';
				p[s.length()+1]='\0';
				int fd=open("group.txt",O_RDWR|O_CREAT|O_APPEND,0666);
				write(fd,p,s.length()+1);
				close(fd);
				send(cc,"OK",2,0);
				memset(&p, '0', sizeof(p)); 
				pthread_mutex_unlock(&l1);
				bool b=true;
				string u="",g="";
				for(int i=0;i<s.length();i++)
				{
					if(s[i]==':')
					b=false;
					else if(b==true)
					g=g+s[i];
					else if(b==false)
					u+=s[i];
				}	
				group.insert(make_pair(g,u));
				char t[10]={0};
				strcpy(t,u.c_str());
				int fd1=open(t,O_RDWR|O_CREAT|O_APPEND,0666);
				g=g+":"+u+":"+"0";	
				char tt[20]={0};				
				strcpy(tt,g.c_str());
				tt[g.length()]='\n';
				tt[g.length()+1]='\0';
				write(fd1,tt,g.length()+1);
				close(fd1);
			}
		}
		else if(opt1=="list_group")
		{
			if(loginC[cc]==true)
			{
				struct stat file;
				int size;
				stat("group.txt",&file);
				size=file.st_size;
				send(cc, &size, sizeof(int),0);
				if(size!=0)
				{
					fflush(stdout);
					int fd=open("group.txt",O_RDONLY);
					sendfile(cc,fd,NULL,size);
					fflush(stdout);
					close(fd);
				}
				cout<<"\nDone\n";
			}
		}
		else if(opt1=="join_group")
		{
			if(loginC[cc]==true)
			{
				char p[100]={0};
				sleep(1);
				recv(cc,p,100,0);
				string s(p);
				auto it=group.find(s);
				if(it!=group.end())
				{
					char p[100]={0};
					strcpy(p,(it->second).c_str());
					auto ii=c_id.find(to_string(cc));				
					string f=it->first+":"+ii->second+":1";
					int fd=open(p,O_RDWR|O_CREAT|O_APPEND,0666);
					strcpy(p,f.c_str());
					p[f.length()]='\n';
					p[f.length()+1]='\0';
					write(fd,p,f.length()+1);
					close(fd);
				}
			}
		}
		else if(opt1=="list_request")
		{
			if(loginC[cc]==true)
			{
				char p[100]={0};
				auto ii=c_id.find(to_string(cc));				
				string f=ii->second;
				strcpy(p,f.c_str());
				struct stat file;
				int size;
				stat(p,&file);
				size=file.st_size;
				send(cc, &size, sizeof(int),0);
				if(size!=0)
				{
					int fd=open(p,O_RDWR|O_CREAT|O_APPEND,0666);
					sendfile(cc,fd,NULL,size);
					fflush(stdout);
					close(fd);
				}
			}
		}
		else if(opt1=="accept_request")
		{
			if(loginC[cc]==true)
			{
				char p[100]={0},pp[100]={0};
				sleep(1);
				recv(cc,p,100,0);
				string s(p);
				cout<<s<<endl;
				bool b=true;
				string u="",g="";
				for(int i=0;i<s.length();i++)
				{
					if(s[i]==':')
					b=false;
					else if(b==true)
					g=g+s[i];
					else if(b==false)
					u+=s[i];
				}
				cout<<g<<" "<<u<<endl;
				int k=0;
				char c;
				auto it=group.find(g);
				string s1="",s2="";
				strcpy(p,it->second.c_str());
				int fd=open(p,O_RDWR|O_CREAT|O_APPEND,0666);
				int fd2=open("temp.txt",O_RDWR|O_CREAT|O_APPEND,0666);
				while((read(fd,&c,1)>0))
				{
					if(c==':')
					{
						k++;
						write(fd2,&c,1);
					}
					else if(c=='1'&&k==2)
					{
						if(s1==g&&s2==u)
						{
							c='0';
							write(fd2,&c,1);
						}
						else
						{
							write(fd2,&c,1);
						}
					}
					else if(c=='\n')
					{
						write(fd2,&c,1);
						s1="";
						s2="";
						k=0;
					}
					else if(k==0)
					{
						s1+=c;
						write(fd2,&c,1);
					}
					else if(k==1)
					{
						s2+=c;
						write(fd2,&c,1);
					}
					else
					{
						write(fd2,&c,1);
					}	
				}
				close(fd);
				close(fd2);
				rename("temp.txt",p);
			}
		}
		else if(opt1=="upload_file")
		{
			char p[100]={0};
			sleep(1);
			recv(cc,p,100,0);
			bool bb=true;
			string a(p),aa="",g="";
			for(int i=0;i<a.length();i++)
			{
				if(bb==true&&a[i]==':')
				{
					bb=false;
				}
				else if(bb==true)
				{
					g+=a[i];
				}
				else
				aa+=a[i];
			}
			cout<<g<<" "<<aa<<endl;
			auto it=group.find(g);
			auto ii=c_id.find(to_string(cc));
			char file[50]={0};
			strcpy(file,it->second.c_str());
			int fd=open(file,O_RDWR,0666);
			char c,cc1=1;int b=0;
			string gg="",u="";
			while((read(fd,&c,1)>0))
			{
				if(c==':')
				{
					b++;
				}
				else if(b==2&&(c=='1'||c=='0'))
				cc1=c;
				else if(c=='\n'&&cc1=='0')
				{
					if(u==ii->second&&g==gg)
					{
						cout<<u<<endl<<ii->second<<endl;
						aa=u+":"+aa;
						char h[100]={0},kk1[100];
						strcpy(h,g.c_str());
						strcpy(kk1,aa.c_str());
						kk1[aa.length()]='\n';
						kk1[aa.length()+1]='\0';
						int fd1=open(h,O_RDWR|O_CREAT|O_APPEND,0666);
						write(fd1,kk1,aa.length()+1);
						close(fd1);
						break;
					}
					gg="";
					b=0;
					u="";
				}
				else if(b==0)
				gg=gg+c;
				else if(b==1)
				u=u+c;
			}
			close(fd);	
		}
		else if(opt1=="download_file")
		{
			string g="",f="";
			bool bb=true;
			char p[100]={0};
			recv(cc,p,100,0);
			for(int i=0;p[i]!='\0';i++)
			{
				if(p[i]==':')
				bb=false;
				else if(bb==true)
				g+=p[i];
				else if(bb==false)
				f+=p[i];
			}
			char h[20]={0};
			strcpy(h,g.c_str());
			auto it=group.find(g);
			auto ii=c_id.find(to_string(cc));
			cout<<g<<" "<<f<<" "<<it->second<<endl;
			char file[50]={0};
			strcpy(file,it->second.c_str());
			int fd=open(file,O_RDWR,0666);
			char c,cc1=1;int b=0,l=1;
			string gg="",u="";
			while((read(fd,&c,1)>0))
			{
				if(c==':')
				{
					b++;
				}
				else if(b==2&&(c=='1'||c=='0'))
				cc1=c;
				else if(c=='\n'&&cc1=='0')
				{
					if(u==ii->second&&gg==g)
					{
						l=0;
						break;
					}
					gg="";
					b=0;
					u="";
				}
				else if(b==0)
				gg=gg+c;
				else if(b==1)
				u=u+c;
			}
			if(l==0)
			{
				send(cc,"0",1,0);
				string user="",name="",size="",sha="";
				char r[10]={0};
				strcpy(r,g.c_str());
				int fd=open(r,O_RDWR,0666);
				char c;	
				int num=0;			
				while(read(fd,&c,1)>0)
				{
					if(c==':')
					num++;
					else if(c=='\n')
					{
						num=0;
						if(name==f)
						{
							user+=":"+name+":"+size+":"+sha;
							char t[100]={0};
							strcpy(t,user.c_str());
							send(cc,t,user.length(),0);
							break;
						}
						user="";
						name="";
						size="";
						sha="";
					}
					else if(num==0)
					{
						user+=c;
					}
					else if(num==1)
					{
						name+=c;
					}
					else if(num==2)
					{
						size+=c;
					}
					else if(num==3)
					{
						sha+=c;
					}
				}
				cout<<"Done\n";
			}
			else
			{
				send(cc,"1",1,0);
			}
		}
		else if(opt1=="logout")
		{
			for(auto it=c_id.begin();it!=c_id.end();it++)
			{
				if(it->second==to_string(cc))
				{
					c_id.erase(it);
					break;
				}
			}
			loginC[cc]=false;
			pthread_exit(NULL);
		}
		ii++;
	}		
}
int main(int argc, char *argv[])
{
	signal(SIGINT,handle);
	signal(SIGPIPE, sigpipe_handler);
	set_User();
	struct sockaddr_in serv_addr; 
   	listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int port;
	sscanf(argv[1], "%d", &port);
	serv_addr.sin_port = htons(port); 
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
	listen(listenfd, 10); 
	int i=0;
	if(pthread_mutex_init(&lock, NULL) != 0) 
	{ 
		printf("\n mutex init has failed\n"); 
		return 1; 
	} 
	if(pthread_mutex_init(&l1, NULL) != 0) 
	{ 
		printf("\n mutex init has failed\n"); 
		return 1; 
	}
	if(pthread_mutex_init(&l2, NULL) != 0) 
	{ 
		printf("\n mutex init has failed\n"); 
		return 1; 
	}
	while(1)
	{
		connfd[ci] = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		pthread_create(&thread[ii],NULL,&menu,(void *)connfd[ci]);
		ci++;
		ii++;
	}
	for(int i=0;i<10;i++)	
	close(connfd[i]);
	pthread_mutex_destroy(&lock);
return 0;
}

