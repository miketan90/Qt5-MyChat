#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <jsoncpp/json/json.h>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <mutex>
#include <atomic>
#include "chatDatabase.h"
#include "flag.h"
//#include "chatinfo.h"


using namespace std;

#define ANYIP "0.0.0.0"
#define MYIP "192.168.130.130"
#define PORT 8000

// 字符串分割
void mysplit(const string str, const char delim, vector<string> &res);

// 回调函数参数
struct Args {
    int fd;
    //struct evconnlistener *listener;
    struct sockaddr *addr;
};
// 数据分包
struct DataPack {
    int head;           // 包头，包体长度
    char body[1024];    // 包体
};


class Server {
private:
    // 事件集合
    struct event_base *base;
    // 监听事件
    struct evconnlistener *listener;
    // 信息链表
    //ChatInfo *chatlist;
    // mysql
    static ChatDatabase *chatdb;

    
private:
    // 监听回调
    static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
    // 多线程回调
    static void client_handler(void *arg);
    // bufferevent读回调
    static void read_cb(struct bufferevent *bev, void *ctx);
    // bufferevent退出事件回调
    static void event_cb(struct bufferevent *bev, short what, void *ctx);

    // 注册业务
    static void server_register(struct bufferevent *bev, const Json::Value & val);
    // 登录业务
    static void server_login(struct bufferevent *bev, const Json::Value & val);
    // 获取用户数据
    static void get_user_data(const Json::Value & val, Json::Value & res);
    // 转发消息实现客户端间通信
    static void transferMsg(struct bufferevent *bev, Json::Value & val);
    // 修改好友业务
    static void friendoperate(struct bufferevent *bev, const Json::Value & val);
    // 发送离线消息
    static void send_offline_msg(struct bufferevent *bev);
    // 上线提示
    static void online_tips(int,string s = "");

public:
    Server(const char *ip = "127.0.0.1", int port = 8000);
    ~Server();
    
};

#endif