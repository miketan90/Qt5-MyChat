### 基于Qt5、TCP通信的即时聊天软件 (客户端 + 服务器)

### 1. 环境
- 客户端：Windows 10 | Qt 5.14.2
- 服务器：Ubuntu 16.04

### 2. 概述
- **服务器**：基于 **libevent** + 多线程 + **mysql** 搭建，主要业务有注册、登录、聊天、离线消息、搜索添加删除好友、好友上线显示。
- **客户端**：TCP通信，主要功能有注册、登录、聊天、收发离线消息、搜索添加删除好友、验证添加好友请求、好友在线显示。

**2.1 聊天主要思路**

客户端A给客户端B发送一条消息
1. 客户端B在线
    - 首先客户端A把消息发给服务器
    - 服务器接收并处理
    - 服务器处理完成，返回处理成功
    - 消息处理后得知消息发给客户端B，判断B**在线**
    - 将消息转发给客户端B
    - 客户端B收到消息，响应接收成功
2. 客户端B不在线
    - 首先客户端A把消息发给服务器
    - 服务器接收并处理
    - 服务器处理完成，返回处理成功
    - 消息处理后得知消息发给客户端B，判断B**不在线**
    - 将消息**写入离线缓存(mysql数据库)**
    - **若客户端B上线，先查找有无客户端B的离线消息**
    - 如果有，读取并发送给客户端B，然后**删除离线缓存中的数据**

**2.2 好友验证主要思路**

客户端A请求客户端B添加好友
1. 客户端B在线
    - 首先客户端A把好友请求发给服务器
    - 服务器接收并处理，判断为未验证消息
    - 再判断B**在线**，将好友请求转发给客户端B
    - 客户端B收到验证请求，客户自行验证后，响应服务器
    - 服务器接收并处理，判断为已验证消息，并判断验证结果
    - 验证为同意：客户端B立即在界面中添加客户端A，与此同时服务器操作添加好友，并响应客户端A验证结果为同意，客户端A在界面中添加客户端B
    - 验证为拒绝：服务器响应客户端A验证结果为拒绝，客户端A在界面提示客户端B拒绝
2. 客户端B不在线
    - 首先客户端A把好友请求发给服务器
    - 服务器接收并处理，判断为未验证消息
    - 再判断B**不在线**，将好友请求**写入离线缓存(mysql数据库)**
    - **若客户端B上线，先查找有无客户端B的离线好友请求**
    - **如果有，读取并发送给客户端B，然后删除离线缓存中的数据**
    - 客户端B收到验证请求，客户自行验证后，响应服务器
    - 服务器接收并处理，判断为已验证消息，并判断验证结果
    - 验证为同意：客户端B立即在界面中添加客户端A，与此同时服务器操作添加好友，并响应客户端A验证结果为同意，客户端A在界面中添加客户端B
    - 验证为拒绝：服务器响应客户端A验证结果为拒绝，客户端A在界面提示客户端B拒绝

**2.3 在线提示思路**
上线或下线向好友列表群发在线状态，客户端根据这个状态设置好友名字的颜色，例如：在线名字为蓝色，不在线名字为黑色。然后更新在线人数


### 3.效果
1. 登录和注册
<center class="half">
<img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C7.jpg" width = "48%" height = "48%" >  <img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C8.jpg" width = "48%" height = "48%" >
</center>

2. 主界面
<img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C1.jpg" width = "100%" height = "100%" >

3. 聊天
<img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C2.jpg" width = "100%" height = "100%" >
<img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C6.jpg" width = "100%" height = "100%" >

4. 好友操作 
<center class="half">
<img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C4.jpg" width = "38%" >  <img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C5.jpg" width = "30%" >  <img src="https://github.com/miketan90/Qt5-MyChat/blob/master/img/%E6%95%88%E6%9E%9C3.jpg" width = "30%" >
</center>
