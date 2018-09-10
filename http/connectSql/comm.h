#pragma once
#include<iostream>
#include<string>
#include<mysql.h>

void test();
MYSQL* mysql_connect();
int insertMysql(MYSQL *myfd,const std::string &name,const std::string &school,const std::string &hobby);
void closeMysql(MYSQL *myfd);
void selectMysql(MYSQL *myfd);
