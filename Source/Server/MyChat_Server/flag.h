#ifndef FLAG_H
#define FLAG_H

enum _cmd{REGISTER,LOGIN,CHAT,FRIEND,UPDATA,ONLINE};
enum _operate{SEARCH,ADD,DEL,ONL,OFFL};

#define _MYSQL_ERROR                -1

#define REGISTER_SUCCESS            1
#define REGISTER_USER_EXIST         0

#define LOGIN_SUCCESS               1
#define LOGIN_ALREADY_DO            2
#define LOGIN_PASSWORD_ERROR        3
#define LOGIN_USER_NONEXISTENCE     4


#endif