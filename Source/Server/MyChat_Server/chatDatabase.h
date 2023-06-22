#ifndef CHATDATABASE_H
#define CHATDATABASE_H

#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <stdio.h>
#include <random>
#include <string.h>
#include <vector>
#include <sstream>
#include <jsoncpp/json/json.h>
#include "flag.h"

using namespace std;

class ChatDatabase {
private:
    MYSQL *mysql;

    static MYSQL_RES * get_MYSQL_RES(MYSQL *mysql, const char *sqlscript);

    void make_uid(string &);
public:
    ChatDatabase();
    ~ChatDatabase();

    // 连接数据库
    void my_database_connect(const char *);
    // 获取一行或多行查询结果, 单行或单列返回1,多行多列返回2, 无数据返回0
    int my_database_get_data(vector<string> & array, const char *sqlscript);
    // 断开数据库连接
    void my_database_disconnect() { mysql_close(this->mysql); }
    // 查找用户是否存在, 如果存在返回ture,通过参数返回用户uid
    bool my_database_user_exist(const string &,string * retuid = nullptr);
    // 用户注册业务
    int my_database_user_register(string name, string password);
    // 用户登录密码验证业务
    int my_database_user_confirm(string name, string password);
    // 添加好友
    void my_database_DML_friend(string sender, string receiver, int mode);
    // 添加离线消息
    void my_database_write_offlineMsg(const Json::Value & val);
    // 读出离线消息并删除
    int my_database_read_offlineMsg(vector<string> &,string);
};


#endif