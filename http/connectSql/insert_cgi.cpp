#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"comm.h"
int main(){
		test();
		char arg[1024];
		if(getenv("METHOD")){
			if(strcasecmp(getenv("METHOD"),"GET")){
				strcpy(arg,getenv("QUERY_STRING"));
			}else{
				char c;
				int i=0;
				int len = atoi(getenv("CONTENT_LENGTH"));
				for(;i<len;i++){
					read(0,&c,1);
					arg[i] = c;
				}
				arg[i] = 0;
			}
		}
		//arg
		//name=""&school""&hobby=""
		//const std::string name="xxx";
		//const std::string school="xaxu";
		//const std::string hobby="xixixi";
		strtok(arg,"=&");
		std::string name = strtok(NULL,"=&");
		strtok(NULL,"=&");
		std::string school = strtok(NULL,"=&");
		strtok(NULL,"=&");
		std::string hobby = strtok(NULL,"=&");

		MYSQL* myfd = mysql_connect();
	  
		insertMysql(myfd,name,school,hobby);
		
		closeMysql(myfd);
		return 0;
	}
