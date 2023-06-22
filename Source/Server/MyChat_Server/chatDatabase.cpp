#include "chatDatabase.h"

ChatDatabase::ChatDatabase() {
    mysql = nullptr;
}

ChatDatabase::~ChatDatabase() {

}

void ChatDatabase::make_uid(string &uid) {

    srand(time(0));

    char tmp = 0;
    for (int i = 0; i < 6; ++i){
        int mode = random() % 2;
        switch(mode){
            case 0:
                tmp = random() % 10;
                tmp += '0';           
                break;
            case 1:
                tmp = random() % 26;
                tmp += 'A';
                break;
            default:
                tmp = random() % 26;
                tmp += 'a';
                break;
        }
        uid += tmp;
    }

}

void ChatDatabase::my_database_connect(const char *db_name) {

    this->mysql = mysql_init(NULL);

    this->mysql = mysql_real_connect(this->mysql,"localhost","root","123456",
                                    db_name,0,NULL,0);
    if (NULL == this->mysql){
        cout << "mysql_real_connect error" << endl;
    }

}

MYSQL_RES * ChatDatabase::get_MYSQL_RES(MYSQL *mysql, const char *sqlscript){

    if (mysql_query(mysql,sqlscript) != 0) {
        cout << "mysql_query error" << endl;
        return nullptr;
    }

    MYSQL_RES *res = mysql_store_result(mysql);
    if (NULL == res) {
        cout << "mysql_store_result error" << endl;
        return nullptr;
    } else {
        return res;
    }
}

int ChatDatabase::my_database_get_data(vector<string> & array, const char *sqlscript) {

    MYSQL_RES *res = get_MYSQL_RES(this->mysql, sqlscript);
    if (NULL == res) {
        return -1;
    }

    int column = mysql_num_fields(res);
    int rn = mysql_num_rows(res);
    //cout << "column: " << column << endl; 

    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
 
        string tmp;
        if(column == 1){
            tmp += row[0];       // 只有一列数据，push进vector里，vector的 索引 是 行
            array.push_back(tmp);
        } else {
            if (rn == 1) {      // 只有一行数据，push进vector里，vector的 索引 是 列
                for (int i = 0; i < column; ++i) {
                    tmp += row[i];
                    array.push_back(tmp);
                    tmp.clear();
                }

            } else {            // 多行多列，vector的 索引 是 行，每列用 '|' 隔开
                for (int i = 0; i < column; ++i) {
                    tmp += row[i];  
                    tmp += '|';
                }
                array.push_back(tmp);
            }        
        } 
        
    }

    if (rn == 1 || column == 1)
        return 1;
    else if (rn == 0)
        return 0;
    else 
        return 2;
}

bool ChatDatabase::my_database_user_exist(const string &user, string * retuid) {

    ostringstream buffer;
    buffer << "select uid from chatuser where username = '"<< user <<"';";
    string statement = buffer.str();

    MYSQL_RES *res = get_MYSQL_RES(this->mysql, statement.c_str());
    if (NULL == res) {
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if(NULL == row)
        return false;
    else{
        if (retuid != nullptr)
            *retuid = row[0];
        return true;
    }     
}

int ChatDatabase::my_database_user_register(string name, string password) {

    // 随机生成不重复的用户uid
    string uid;      
    make_uid(uid); //随机产生6位数字

    ostringstream buffer;
    buffer << "select uid from chatuser where uid = '"<< uid <<"';";
    string statement = buffer.str();

    MYSQL_RES *res = get_MYSQL_RES(this->mysql, statement.c_str());
    if (res == NULL)
        return -1;

    while (mysql_num_rows(res) != 0) {
        // 有重复就清除重新生成
        uid.clear();    
        make_uid(uid);

        buffer.str("");
        buffer << "select uid from chatuser where uid = '"<< uid <<"';";
        string s = buffer.str();

        res = get_MYSQL_RES(this->mysql, s.c_str());
    }


    // 注册用户数据
    buffer.str("");
    statement.clear();
    buffer << "insert into chatuser(username,userpassword,uid) " 
            << "values ('"<< name <<"','"<< password <<"','"<< uid <<"');";
    statement = buffer.str();

    if(0 != mysql_query(this->mysql, statement.c_str())){
        cout << "mysql_query error" << endl;
        return -1;
    }

    return 0;
}

int ChatDatabase::my_database_user_confirm(string name, string password) {

    ostringstream buffer;
    buffer << "select userpassword from chatuser where username = '"<< name << "';";
    string statement = buffer.str();


    MYSQL_RES *res = get_MYSQL_RES(this->mysql, statement.c_str());
    if (NULL == res) {
        //return string("");
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if(row == nullptr)
        //return string("none");
        return 2;

    if(password == row[0])
        return 1;
    else 
        return 0;
}

void ChatDatabase::my_database_DML_friend(string sender, string receiver, int mode){

    ostringstream buf;
    buf << "select id from chatuser where uid = '"<< sender <<"' or uid = '"<< receiver <<"';";
    string statement = buf.str();

    MYSQL_RES * res = get_MYSQL_RES(mysql, statement.c_str());

    MYSQL_ROW row; // 按上面的sql语句搜索一定有两行，不存在的用户在搜索阶段就排除了
    row = mysql_fetch_row(res);
    string first = row[0];
    row = mysql_fetch_row(res);
    string second = row[0];

    buf.str("");
    statement.clear();
    if (mode == ADD)
        buf << "insert into user_friend(usr_id,friend_id) value ("<< first <<","<< second <<");";
    else if (mode == DEL)
        buf << "delete from user_friend where (usr_id = "
            << first <<" and friend_id = "<< second <<") or (usr_id = "
            << second <<" and friend_id = "<< first <<");";
    statement = buf.str();

    if(0 != mysql_query(this->mysql, statement.c_str())){
        cout << "mysql_query error" << endl;
        return;
    }

}

void ChatDatabase::my_database_write_offlineMsg(const Json::Value & val) {

    const char * sender = val["sender"].asCString();
    const char * receiver = val["receiver"].asCString();
    Json::FastWriter writer;
    string msg = writer.write(val);

    ostringstream buf;
    buf << "insert into offline_msg(sender,receiver,msg,time) "
        <<"value ('"<< sender <<"','"<< receiver <<"','"<< msg <<"',now());";
    string statement = buf.str();

    if(0 != mysql_query(this->mysql, statement.c_str())){
        cout << "mysql_query error" << endl;
        return;
    }

}

int ChatDatabase::my_database_read_offlineMsg(vector<string> &vec,string receiver) {

    ostringstream buf;
    buf << "select msg,time from offline_msg where receiver = '"<< receiver <<"';";
    string stmt = buf.str();

    int ret = my_database_get_data(vec, stmt.c_str());

    if (ret > 0) {
        buf.str("");
        buf << "delete from offline_msg where receiver = '"<< receiver <<"';";
        stmt.clear();
        stmt = buf.str();
        if(0 != mysql_query(this->mysql, stmt.c_str())){
            cout << "mysql_query error" << endl;
            return -1;
        }

    }

    return ret;

}