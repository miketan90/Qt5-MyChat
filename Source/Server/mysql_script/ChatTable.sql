use MyGroup;

show tables;

create table GrpName (
	id int primary key auto_increment,
    name varchar(32)
)comment '群名称表';

create table GrpMember (
	id int primary key auto_increment,
    name varchar(32),
    isLeader bool,
    dept_id int,
    constraint fk_dept_id foreign key(dept_id) references GrpName(id)
) comment '群成员表';

insert into GrpName value (1,'学习交流群1'),(2,'学习交流群2'),(3,'学习交流群3');
insert into GrpMember(name,isLeader,dept_id) value ('小张',true,1),('小刘',false,1),('小王',false,1),('小李',false,1),('小孟',false,1);
insert into GrpMember(name,isLeader,dept_id) value ('小刘',true,2),('小孙',false,2),('小红',false,2),('小明',false,2),('小李',false,2),('小赵',false,2);
insert into GrpMember(name,isLeader,dept_id) value ('小冯',true,3),('小费',false,3),('小花',false,3),('小张',false,3);

select * from GrpName;
select * from GrpMember;

--------------------------------------------------- 以上表格不用管，因为没实现群聊 ---------------------------------------------------

use MyUser;

create table chatuser(
	id int primary key auto_increment,
    UID char(6) unique not null,
    username varchar(32),
    userpassword varchar(16)
) comment '用户表';

insert into chatuser(UID,username,userpassword) value ('9684ab','小明','a123456');

select * from chatuser;



create database UserOfflineMsg;

use UserOfflineMsg;

create table offline_msg(
	id int auto_increment primary key,
    sender char(6) not null,
    receiver char(6) not null,
	msg text,
    time datetime
) comment '离线消息表';

drop table offline_msg;

insert into offline_msg(sender,receiver,msg,time) value ('test01','test02','123456789',now());
select * from offline_msg;

select msg,time from offline_msg where receiver = 'test02';
delete from offline_msg where receiver = 'test02';