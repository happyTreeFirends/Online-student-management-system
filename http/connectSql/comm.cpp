#include"comm.h"
void test(){
		std::cout<<"client version: "<<mysql_get_client_info()<<std::endl;
	}

MYSQL* mysql_connect(){
	MYSQL *myfd = mysql_init(NULL);
	if(mysql_real_connect(myfd,"127.0.0.1","root","","rocket",3306,NULL,0)==NULL){
			std::cout<<"connect error"<<std::endl;
		}else{
			std::cout<<"connect success"<<std::endl;
		}
		return myfd;
}
int insertMysql(MYSQL* myfd,const std::string &name,const std::string &school,const std::string &hobby)
{
	//VALUES("zhangsan","xatu","coding")
	std::string sql="INSERT INTO students (name,school,hobby) VALUES(\"";
	sql +=name;
	sql +="\",\"";
	sql += school;
	sql +="\",\"";
	sql +=hobby;
	sql +="\")";

	std::cout<<sql<<std::endl;

	return mysql_query(myfd,sql.c_str());
}

void selectMysql(MYSQL *myfd)
{
	std::string sql="SELECT * FROM students";
	mysql_query(myfd,sql.c_str());

	MYSQL_RES * result = mysql_store_result(myfd);
	int lines = mysql_num_rows(result);
	int cols = mysql_num_fields(result);

	MYSQL_FIELD *field = mysql_fetch_fields(result);
	int i = 0;
	for(;i<cols;i++)
	{
		std::cout<<field[i].name<<'\t';
	}
	std::cout<<std::endl;

	for(i=0;i<lines;i++)
	{
		MYSQL_ROW line = mysql_fetch_row(result);
		int j = 0;
		for(;j<cols;j++)
			{
				std::cout<<line[j]<<'\t';
			}
			std::cout<<std::endl;
	}
	free(result);
}


void closeMysql(MYSQL *myfd)
{
	mysql_close(myfd);	
}
