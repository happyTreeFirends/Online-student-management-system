#include<stdio.h>
#include<pthread.h>
#include<fcntl.h>
#include<string.h>
#include<signal.h>
#include<assert.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<sys/sendfile.h>

#define MAX 1024
#define HOME_PAGE "index.html"
#define PAGE_404 "wwwroot/404.html"

static void usage(const char *proc)   // zai ben han shu nei bu you xiao
{
	printf("Usage: %s port\n\n",proc);
}

static int getLine(int sock,char line[],int num)
{
	assert(line);
	assert(num>0);
	char c='x';
	int i=0;
	while(c!='\n'&&i<num-1)//num yu liu '\0'
	{
		ssize_t s=recv(sock,&c,1,0);//tcp du qu zhuan yong jie kou recv 
		if(c>0)
		{
			if(c=='\r')
			{
				recv(sock,&c,1,MSG_PEEK);
				if(c=='\n')
				{
					recv(sock,&c,1,0);
				}else{
					c='\n';
				}
			}
			line[i++]=c;
		}else{
			break;
		}
	}
	line[i]=0;
	return i;
}
void clearHeader(int sock)
{
	char line[MAX];
	do{
	getLine(sock,line,sizeof(line));
	printf("%s",line);
	}while(strcmp(line,"\n")!=0);
}

void show_404(int sock){
		char line[1024];
		sprintf(line,"HTTP/1.0 404 Not Found\r\n");
		send(sock,line,strlen(line),0);
		sprintf(line,"Content-Type:text/html;charset=ISO-8859-1\r\n");
		send(sock,line,strlen(line),0);
		sprintf(line,"\r\n");
		send(sock,line,strlen(line),0);
	
		struct stat st;
		stat(PAGE_404,&st);
		int fd = open(PAGE_404,O_RDONLY);
		sendfile(sock,fd,NULL,st.st_size);
		close(fd);
	}

void echoError(int sock,int status_code)
{
	switch(status_code)
	{
		case 400:
			//show_404;
			break;
		case 403:
			break;
		case 404:
			show_404(sock);
			break;
		case 500:
			break;
		case 503:
			break;
defalt:
			break;
	}
}

int echo_www(int sock,char *path,int size)
{
	clearHeader(sock);
	int fd = open(path,O_RDONLY);
	if(fd<0)
	{
		return 404;	
	}
	
	char *stuff = path + strlen(path)-1;
	while(*stuff != '.' && stuff != path){
			stuff--;
		}

	char line[MAX];
	sprintf(line,"hTTP/1.0 200 ok\r\n");
	send(sock,line,strlen(line),0);

	if(strcmp(stuff,".html")==0){
		sprintf(line,"Content-Type: text/html\r\n");
		}
		else if(strcmp(stuff,".css")==0){
			sprintf(line,"Content-Type: text/css\r\n");
			}
			else if(strcmp(stuff,".js")==0){
				sprintf(line,"Content-Type: application/x-javascript\r\n");
				}
	send(sock,line,strlen(line),0);

	sprintf(line,"Content - Type: text/html;charset=ISO-8859-1\r\n");
	send(sock,line,strlen(line),0);

	sprintf(line,"\r\n");
	send(sock,line,strlen(line),0);
	
	sendfile(sock,fd,NULL,size);

	close(fd);
	return 200;
}

int exe_cgi(int sock,char *method,char *path,char *query_string)
{
		char line[MAX];
		char method_env[MAX/10];
		char query_string_env[MAX];
		char content_length_env[MAX/10];
		int content_length=-1;
		if(strcasecmp(method,"GET")==0){
			clearHeader(sock);
		}else{
			do{
					getLine(sock,line,sizeof(line));
					if(strncmp(line,"Content-Length: ",16)==0){
								content_length = atoi(line+16);
							}
			 }while(strcmp(line,"\n"));
			if(content_length == -1){
				return 400;
				}
			}
			int input[2];//child 角度
			int output[2];
			
			pipe(input);
			pipe(output);

			//zhi xing path
			pid_t id = fork();
			if(id<0){
				return 503;
				}else if(id==0){
					//child
					close(input[1]);
					close(output[0]);

					sprintf(method_env, "METHOD=%s",method);
					putenv(method_env);
				
					if(strcasecmp(method,"GET")==0){
						sprintf(query_string_env,"QUERY_STRING=%s",query_string);
						putenv(query_string_env);
					}else{
							sprintf(content_length_env,"CONTENT_LENGTH=%d",content_length);
							putenv(content_length_env);//被替换的子进程能拿到2个参数,get方法string put拿length

					}

					dup2(input[0],0);
					dup2(output[1],1);
					//exec*
					execl(path,path,NULL);


					exit(1);	
				}else{
						close(input[0]);
						close(output[1]);
						int i = 0;
						char c;
						if(strcasecmp(method,"POST")==0){
							for(;i<content_length;i++){
									recv(sock,&c,1,0);
									write(input[1],&c,1);
								}
							}
						sprintf(line,"HTTP/1.0 200 OK\r\n");
						send(sock,line,strlen(line),0);
						sprintf(line,"Content-Type: text/html;charset=ISO-8859-1\r\n");
						send(sock,line,strlen(line),0);
						sprintf(line,"\r\n");
						send(sock,line,strlen(line),0);
						
						while(read(output[0],&c,1)>0){
							send(sock,&c,1,0);
						}

						waitpid(id,NULL,0);
						close(input[1]);
						close(output[0]);
				}
	return 200;
}


void *handlerRequest(void *arg)
{
	int sock=(int)arg;
	char line[MAX];
	char method[MAX/16];
	char url[MAX];
	char path[MAX];
	int i,j;
	int status_code=200;
	int cgi=0;
	char *query_string = NULL;


	getLine(sock,line,sizeof(line));//an hang chu li  line -> \r \r\n \n  ->\n bu chu li hui chu xian nian bao wen ti

	printf("line: %s\n",line);
	j=0;
	i=0;
	while(i<sizeof(method)-1&&j<sizeof(line)&&!isspace(line[j]))
	{
		method[i]=line[j];
		i++,j++;
	}
	method[i]='\0';
//get fang fa yong url chuan can     POST tong guo qint qiu zheng wen chuan can 
//POST chuan can cai you zheng wen 
if(strcasecmp(method,"GET")==0)
	{}
	else if(strcasecmp(method,"POST")==0)// hu lue da xiao xie de bi jiao
	{
		cgi=1;//POST fang fa bi xu yi cgi fang shi yun xing 
	}else
	{
		status_code=400;
		clearHeader(sock);
		goto end;
	}
	while(j<sizeof(line)&&isspace(line[j])){
		j++;
	}
	i=0;
	while(i<sizeof(url)-1&&j<sizeof(line)&&!isspace(line[j])){
		url[i]=line[j];
		i++,j++;
	}
	url[i]='\0';

#ifdef DEBUG   //tiao jian bian yi
	printf("method: %s,url:%s\n",method,url);
#endif

//method url cgi
	if(strcasecmp(method,"GET")==0){
		query_string = url;
		while(*query_string!='\0'){
			if(*query_string=='?'){
				cgi=1;
				*query_string='\0';
				query_string++;
				break;
				}
				query_string++;
			}
		}
		printf("hello 263\n");
	//method GET/POST, url ,GET(query_string),cgi
	sprintf(path,"wwwroot%s",url);//path(wwwroot/,wwwroot/a/b/c,html)//qu chu zi yuan 
	if(path[strlen(path)-1]=='/'){
			strcat(path,HOME_PAGE);
		}
	printf("method: %s\n",method);
	printf("path: %s\n",path);
	printf("cgi: %d\n",cgi);
	printf("query_string: %s\n",query_string);
	
	struct stat st;
	if(stat(path,&st)<0){
			clearHeader(sock);
			status_code = 404;
			goto end;
		}else{
			if(S_ISDIR(st.st_mode)){
					strcat(path,HOME_PAGE);
				}else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH )){
						cgi=1;
				}else{
					//zi yuan ken ding cun zai    zi yuan ken ding bu shi wen jian    zi yuan ken ding mei you  ke zhi xing cheng xu 	
				
				}
				if(cgi){
					//zhi xing cgi
				status_code =  exe_cgi(sock,method,path,query_string);
				}else{
				status_code = echo_www(sock,path,st.st_size);
				}
		}

end:
	if(status_code!=200){
		echoError(sock,status_code);
	}
	close(sock);
}

int startup(int port){
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		perror("socket");
		exit(2);
	}

	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=htonl(INADDR_ANY);
	local.sin_port=htons(port);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0){
		perror("bind");
		exit(3);
	}
	if(listen(sock,10)<0){
		perror("listen");
		exit(4);
	}
	return sock;
}

//./httpd 80   ip dong tai 
int main(int argc,char *argv[])
{
	if(argc!=2){
		usage(argv[0]);
		return 1;
	}
	signal(SIGPIPE,SIG_IGN);
	int listen_sock=startup(atoi(argv[1]));// jian ting tao jie zi
	for(;;){
		struct sockaddr_in client;
		socklen_t len=sizeof(client);
		int sock=accept(listen_sock,(struct sockaddr*)&client,&len);
		if(sock<0){
			perror("accept");
			continue;
		}
		printf("get a new connect...!\n");
		pthread_t tid;
		pthread_create(&tid,NULL,handlerRequest,(void *)sock);
		pthread_detach(tid);
	}
	return 0;
}
