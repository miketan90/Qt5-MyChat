#include "server.h"


// 线程局部变量，记录每个在线用户的uid
thread_local string selfuid;
// 记录朋友
thread_local list<string> myfriend;
// 重复登录flag
thread_local bool IsRepeatLogin = false;
// 在线人员登记
static map<string, struct bufferevent *> online;
mutex g_mutex;

void mysplit(const string str, const char delim, vector<string> & res){

    stringstream s1(str);
    string item;
    while (getline(s1, item, delim)) {
        res.push_back(item);
    }

}


ChatDatabase * Server::chatdb = new ChatDatabase;

Server::Server(const char *ip, int port){

    //this->chatlist = new ChatInfo;

    // 创建事件集合
    this->base = event_base_new();

    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    addr_server.sin_addr.s_addr = inet_addr(ip);

    // 创建监听对象
    listener = evconnlistener_new_bind(base,listener_cb,NULL,
                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,10,
                (struct sockaddr *)&addr_server, sizeof(addr_server));
    if(NULL == listener){
        cout << "evconnlistener_new_bind error\n";
    }

    //监听事件
    event_base_dispatch(base);

}

Server::~Server(){

    //释放
    evconnlistener_free(listener);
    event_base_free(base);

}

void Server::listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg){
    cout << "接收客户端连接, fd = " << fd << endl;

    struct Args args = {0};
    args.fd = fd;
    //args.listener = listener;
    args.addr = addr;

    //创建线程处理客户端
    thread client_thread(client_handler, (void *)&args);
    client_thread.detach(); //线程分离，线程运行结束自动释放资源
}


void Server::client_handler(void *arg){

    struct Args * args = (struct Args *)arg;

    // 创建集合
    struct event_base *base = event_base_new();

    // 创建bufferevent对象
    struct bufferevent *bev = bufferevent_socket_new(base, args->fd, BEV_OPT_CLOSE_ON_FREE);
    if(NULL == bev){
        cout << "bufferevent_socket_new error" << endl;
    }

    // 设置回调
    bufferevent_setcb(bev, read_cb, NULL, event_cb, arg);

    // 使能
    bufferevent_enable(bev, EV_READ);

    event_base_dispatch(base);

    event_base_free(base);
}


void Server::read_cb(struct bufferevent *bev, void *ctx){

    struct Args * args = (struct Args *)ctx;
    
    struct sockaddr_in *addr = (struct sockaddr_in*)(args->addr);
    cout << "From fd : " << args->fd << ", Client addr:port -> " << inet_ntoa(addr->sin_addr)
        << ":" << ntohs(addr->sin_port) << endl;

    char recvbuf[1024] = {0};
    int size = bufferevent_read(bev, recvbuf, sizeof(recvbuf));
    if(size < 0) {
        cout << "bufferevent_read error" << endl;
    }

    cout << "Server recv: " << recvbuf << endl;


    Json::Reader reader;        // 解析json
    Json::Value val;            // 解析的json数据

    //cout << "begin parse" << endl;
    if(!reader.parse(recvbuf, val)){
        cout << "parse failure" << endl;
        return;
    }
    //cout << "parse success" << endl;

    int cmd = val["cmd"].asInt();
    switch(cmd){

    case REGISTER:
        // 注册
        server_register(bev, val);
        break;

    case LOGIN:
        // 登录并获取用户数据
        server_login(bev, val);
        break;

    case CHAT:
        // 用户聊天转发
        transferMsg(bev, val);
        break;

    case FRIEND:
        // 好友操作
        friendoperate(bev,val);
        break;

    case UPDATA:
        // 更新数据库数据
        break;

    default:
        break;

    }

}


void Server::event_cb(struct bufferevent *bev, short what, void *ctx){

    // 清除登录状态
    if (!IsRepeatLogin) {
        auto search = online.find(selfuid);
        if (search != online.end()) {
            lock_guard<mutex> lock(g_mutex);
            online.erase(search);
        }
    } 

    // 告诉在线的好友我下线了
    online_tips(OFFL);
 
    if (what & BEV_EVENT_EOF) {
        cout << "用户:" << selfuid << "下线" << endl;
        bufferevent_free(bev);
    } else {
        printf("未知错误\n");
    }

}


void Server::server_register(struct bufferevent *bev, const Json::Value & val){
    //cout << "server_register" << endl;
    Json::Value response;
    
{
    lock_guard<mutex> lock(g_mutex); // 加锁，保证访问数据库线程安全

    chatdb->my_database_connect("MyUser");

    // 判断用户是否存在
    if(chatdb->my_database_user_exist(val["user"].asString())) {  
        response["cmd"] = REGISTER;
        response["result"] = REGISTER_USER_EXIST;

    } else {
        // 向mysql注册，-1表示sql语句失败了，否则注册成功
        if (-1 == chatdb->my_database_user_register(val["user"].asString(),val["password"].asString())) {
            response["cmd"] = REGISTER;
            response["result"] = _MYSQL_ERROR;
            return;
        } else {
            response["cmd"] = REGISTER;
            response["result"] = REGISTER_SUCCESS;
        }
       
    }

    chatdb->my_database_disconnect();
}
    

    Json::FastWriter writer;    // 封装json
    string str = writer.write(response);
    if (bufferevent_write(bev, str.c_str(), str.length()) < 0) {
        cout << "bufferevent_write error" << endl;
    }

}


void Server::server_login(struct bufferevent *bev, const Json::Value & val) {
    //cout << "server_login" << endl;
    Json::Value response;


    chatdb->my_database_connect("MyUser");

    // 判断用户是否存在
    string retuid;
    if(chatdb->my_database_user_exist(val["user"].asString(), &retuid)){  

        int ret = chatdb->my_database_user_confirm(val["user"].asString(),val["password"].asString());
        if (ret >= 0){
            if (ret == 0){
                response["cmd"] = LOGIN;
                response["result"] = LOGIN_PASSWORD_ERROR;

            } else {
                // 记录该线程用户的uid
                selfuid = retuid;
                // 登录检查，未登录则记录登录信息，登录了则回复已经登录消息
                auto search = online.find(selfuid);
                if (search != online.end()){
                    // 设置重复登录，如果没有这个标志，重复登录两次就能破解重复登录
                    IsRepeatLogin = true;

                    response["cmd"] = LOGIN;
                    response["result"] = LOGIN_ALREADY_DO;

                } else {

                    // 在线登记 
                    lock_guard<mutex> lock(g_mutex);
                    online.insert({selfuid,bev});

                    // 验证成功且未重复登录，请求用户数据
                    get_user_data(val,response);
                    
                    response["cmd"] = LOGIN;
                    response["result"] = LOGIN_SUCCESS;

                }  

            }     

        } else {
            response["cmd"] = LOGIN;
            response["result"] = _MYSQL_ERROR;
        }

    } else {
        
        response["cmd"] = LOGIN;
        response["result"] = LOGIN_USER_NONEXISTENCE;
    }

    chatdb->my_database_disconnect();



    Json::FastWriter writer;    // 封装json
    string str = writer.write(response);
    if (bufferevent_write(bev, str.c_str(), str.length()) < 0) {
        cout << "bufferevent_write error" << endl;
    }


    if (response["result"] == LOGIN_SUCCESS) {
        
        online_tips(ONL);   // 告诉好友我上线了

        send_offline_msg(bev);
    }
  
}


void Server::get_user_data(const Json::Value & val, Json::Value & res){
    //cout << "get_user_data" << endl;

    // 获取用户数据和好友消息
    ostringstream buf;
    buf << "select id,username,uid from chatuser where username = '"<< val["user"].asString() <<"';";
    string stmt = buf.str();

    // 取user的个人数据
    vector<string> user;          
    chatdb->my_database_get_data(user, stmt.c_str());

    buf.str("");
    stmt.clear();

    // 取user的朋友
    vector<string> ufriend;
    buf << "select username,uid from chatuser where id in("
        << "select friend_id as f from user_friend where usr_id = "<< user.at(0)
        << " union all "
        << "select usr_id as f from user_friend where friend_id = "<< user.at(0) <<");";
    stmt = buf.str();

    int ret = chatdb->my_database_get_data(ufriend, stmt.c_str());


    // 封装Json数据
    res["user"] = Json::Value(user.at(1).c_str());
    res["uid"] = Json::Value(user.at(2).c_str());


    Json::Value item;
    if (ret == 1) {       
        item["username"] = ufriend.at(0).c_str();
        item["uid"] = ufriend.at(1).c_str();
        res["friend"].append(item);

        myfriend.push_back(ufriend.at(1));  // 线程私有变量，用于群发


    } else if (ret == 2) {
        for (auto it = ufriend.begin(); it != ufriend.end(); ++it) {

            vector<string> col;      
            mysplit(*it,'|',col);

            item["username"] = col.at(0).c_str();
            item["uid"] = col.at(1).c_str();
            res["friend"].append(item);

            myfriend.push_back(col.at(1));  // 线程私有变量，用于群发

        }
    }

}

void Server::transferMsg(struct bufferevent *bev, Json::Value & val) {
    cout << "transferMsg" << endl;

    // 判断receiver是否在线
    string receiver = val["receiver"].asString();
    auto search = online.find(receiver);

    if (search != online.end()) {
        
        val["isonline"] = true;
        Json::FastWriter writer;
        string str = writer.write(val);
        // 在线则转发到receiver
        if (bufferevent_write(online.at(receiver), str.c_str(), str.length()) < 0) {
        cout << "bufferevent_write error" << endl;
        }
      
    } else {

        // 1.离线消息写入数据库      
{   
        lock_guard<mutex> lock(g_mutex);

        chatdb->my_database_connect("UserOfflineMsg");
        
        chatdb->my_database_write_offlineMsg(val);

        chatdb->my_database_disconnect();
}

        val["msg"] = "offline message";
        val["isonline"] = false;

        Json::FastWriter writer;
        string str = writer.write(val);
        // 2.告诉自己对方不在线
        if (bufferevent_write(bev, str.c_str(), str.length()) < 0) {
        cout << "bufferevent_write error" << endl;
        }
        
    }
   

}

void Server::friendoperate(struct bufferevent *bev, const Json::Value & val) {

    Json::Value response;
    response["cmd"] = FRIEND;
    
    int op = val["operate"].asInt();
    // 查找好友
    if (op == SEARCH) {

        response["operate"] = SEARCH;
        int ret;
        vector<string> tmp;
{       
        lock_guard<mutex> lock(g_mutex);

        chatdb->my_database_connect("MyUser");
        
        ret = chatdb->my_database_get_data(tmp, val["sql"].asCString());

        chatdb->my_database_disconnect();
}      

        // 无数据
        if (ret == 0) {

            if (bufferevent_write(bev, "{\"cmd\":3,\"result\":null}", strlen("{\"cmd\":3,\"result\":null}")) < 0) {
                cout << "bufferevent_write error" << endl;
            }
            return;

        } else {
        // 有数据写入json格式
            if (ret == 1){

                for (auto it = tmp.begin(); it != tmp.end(); ++it){
                    response["result"].append(*it);
                }

            } else if (ret == 2) {
                for (auto it = tmp.begin(); it != tmp.end(); ++it){
                    vector<string> v;
                    Json::Value row;
                    mysplit(*it,'|', v);
                    for (auto vit = v.begin(); vit != v.end(); ++vit){
                        row.append(*vit);
                    }
                    response["result"].append(row);
                }
            }

        }

        // 返回结果
        Json::FastWriter writer;
        string str = writer.write(response);
        if (bufferevent_write(bev, str.c_str(), str.length()) < 0) {
            cout << "bufferevent_write error" << endl;
        }

    // 添加好友
    } else if (op == ADD){

        response["operate"] = ADD;

        string sender = val["sender"].asString();
        string receiver = val["receiver"].asString();
        bool verity = val["verity"].asBool();

        auto it = online.find(receiver.c_str());
        // 对方在线，将好友请求发给对方
        if (it != online.end()){

            Json::FastWriter writer;
            // 发送未验证请求给对方
            if (!verity) {
                
                string str = writer.write(val);
                if (bufferevent_write(online[receiver], str.c_str(), str.length()) < 0) {
                    cout << "bufferevent_write error" << endl;
                }

                // 保证添加者和被添加者线程中 myfriend 都有对方
                // 解决添加好友后不显示下线的bug
                if (find(myfriend.begin(),myfriend.end(),receiver) == myfriend.end()) {
                    // 防止使用拒绝添加好友，使myfriend填满无用重复数据
                    myfriend.push_back(receiver);
                }           

            // 验证后对方回发自己的消息，里面包含同意或拒绝
            } else {
                
                bool IsAgree = val["IsAgree"].asBool();
                if (IsAgree) {            
     
                    lock_guard<mutex> lock(g_mutex);
                    chatdb->my_database_connect("MyUser");

                    chatdb->my_database_DML_friend(sender, receiver, ADD);

                    chatdb->my_database_disconnect(); 
                        
                }


                string str = writer.write(val);
                if (online.find(receiver.c_str()) != online.end()) {
                    if(bufferevent_write(online[receiver], str.c_str(), str.length()) < 0) {
                        cout << "bufferevent_write error" << endl;
                    }
                }

                
                if (IsAgree){
                    myfriend.push_back(receiver);
                    online_tips(ONL,receiver);
                }
            }

        // 对方不在线，将请求送入数据库    
        } else {

            lock_guard<mutex> lock(g_mutex);
            if (verity && val["IsAgree"].asBool()) {      
                
                chatdb->my_database_connect("MyUser");

                chatdb->my_database_DML_friend(sender, receiver, ADD);

                chatdb->my_database_disconnect(); 

                myfriend.push_back(receiver);  
            } 


            chatdb->my_database_connect("UserOfflineMsg");

            chatdb->my_database_write_offlineMsg(val);

            chatdb->my_database_disconnect();    

        }            
   
    // 删除好友
    } else if (op == DEL) {

        // 清理被删除者线程中的myfriend，保证两个线程中的myfriend都没有对方
        // 解决胡乱显示下线的bug
        if (val["clean"].asBool()) {
            myfriend.remove(val["target"].asString());
            return;
        }

        string sender = val["sender"].asString();
        string receiver = val["receiver"].asString();
     
{      
        lock_guard<mutex> lock(g_mutex);
        chatdb->my_database_connect("MyUser");

        chatdb->my_database_DML_friend(sender, receiver, DEL);

        chatdb->my_database_disconnect();

}      
        // 清理删除者线程中的myfriend
        myfriend.remove(receiver);
      
        if (online.find(receiver.c_str()) != online.end()) {

            online_tips(OFFL,receiver);

            Json::FastWriter writer;
            string str = writer.write(val);
            if(bufferevent_write(online[receiver], str.c_str(), str.length()) < 0) {
                cout << "bufferevent_write error" << endl;
            }
        }

    } else{}
  
}

void Server::send_offline_msg(struct bufferevent *bev) {

    vector<string> result;
    int ret;

{      
    lock_guard<mutex> lock(g_mutex);  
    chatdb->my_database_connect("UserOfflineMsg");

    ret = chatdb->my_database_read_offlineMsg(result, selfuid);

    chatdb->my_database_disconnect();
}

    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value val;
    if (ret == 1) {

        reader.parse(result[0],val);
        val["time"] = result[1].c_str();
        val["isonline"] = true;

        string str = writer.write(val);
        if(bufferevent_write(bev, str.c_str(), str.length()) < 0) {
                cout << "bufferevent_write error" << endl;
        }

    } else if (ret == 2) {

        for (auto it = result.begin(); it != result.end(); ++it) {

            vector<string> tmp;
            mysplit(*it,'|',tmp);
            reader.parse(tmp[0],val);
            val["time"] = tmp[1].c_str();
            val["isonline"] = true;           

            string str = writer.write(val);
            if(bufferevent_write(bev, str.c_str(), str.length()) < 0) {
                    cout << "bufferevent_write error" << endl;
            }

        }

    } 

}

void Server::online_tips(int n,string s){
cout << selfuid << " use function: online_tips" << endl;
    // 用于告诉好友我上线了
    Json::Value tell;
    tell["cmd"] = ONLINE; 
    tell["sender"] = selfuid;

    switch(n) {
        case ONL:
        tell["iscome"] = true;
        break;
        
        case OFFL:
        tell["iscome"] = false;
        break;

        default:
        break;
    }

    Json::FastWriter writer;    
    string pack1 = writer.write(tell);

    // 群发
    if (s.empty()) {

        auto my = online.find(selfuid);  // 看看我是不是正在离线，是的话就不用回复我好友在不在线了

        for (auto it = myfriend.begin(); it != myfriend.end(); ++it) {
            // 如果好友在线，告诉这个好友我上线或离线了

            if (online.find(*it) != online.end()) {
                
                if (bufferevent_write(online[*it], pack1.c_str(), pack1.length()) < 0) {
                    cout << "bufferevent_write error" << endl;
                }

                // 同时也返回告诉我这个好友在线，如果我正在离线就不走这里
                if ( my != online.end()) {
                    tell["friend"] = *it;
                    string pack2 = writer.write(tell);
                    if (bufferevent_write(online[selfuid], pack2.c_str(), pack2.length()) < 0) {
                        cout << "bufferevent_write error" << endl;
                    }
                }
            }
            
        }
    // 单发, 用于添加删除好友时，处理好友在线状态
    } else {

        if (online.find(s) != online.end()) {
            if (bufferevent_write(online[s], pack1.c_str(), pack1.length()) < 0) {
                cout << "bufferevent_write error" << endl;
            }

            // 自己客户端点击删除和添加，自己一定是在线状态
            tell["friend"] = s;
            string pack2 = writer.write(tell);
            if (bufferevent_write(online[selfuid], pack2.c_str(), pack2.length()) < 0) {
                cout << "bufferevent_write error" << endl;
            }
        }
    }

}