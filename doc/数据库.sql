
drop table if exists `fx_user`;
create table `fx_user` (
    `id` int primary key auto_increment,
    `name` varchar(32) not null unique comment '登录名',
    `email` varchar(32) comment '电子邮箱',
    `password` varchar(32) not null comment '密码',
    `true_name` varchar(16) comment '真实姓名',
    `department` int not null comment '部门编号',
    `icon` text comment 'base64图片',
    `status` int(1) not null default 1 comment '1可用，0停用',
    `gmt_create` datetime not null comment '创建时间',
    `gmt_modify` datetime not null comment '修改时间'
);

drop table if exists `fx_department`;
create table `fx_department` (
    `id` int primary key auto_increment,
    `parent_id` int not null default 0 comment '上级部门id，0为一级部门',
    `name` varchar(32) not null comment '部门名称',
    `gmt_create` datetime not null comment '创建时间',
    `gmt_modify` datetime not null comment '修改时间'
);

drop table if exists `fx_changelog`;
create table `fx_changelog` (
    `id` int primary key auto_increment,
    `object` varchar(32) not null comment '操作对象，user或department',
    `operation` int(1) not null comment '操作类型，1 delete、2 modify、3 create',
    `time` datetime not null comment '修改时间'
) comment '数据修改日志表，用于客户端更新用户列表';

drop table if exists `fx_chatlog`;
create table `fx_chatlog` (
    `id` int primary key auto_increment,
    `senderid` int not null comment '发送用户id',
    `targetid` int not null comment '接收用户id',
    `msg` text comment '消息主体',
    `gmt_create` datetime not null comment '创建时间',
    `status` int(1) not null default 0 comment '消息状态，0未接收 1已接收'
) comment '聊天记录表';
