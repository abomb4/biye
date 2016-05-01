-- SQLite3 db
drop table if exists `fx_user`;
create table `fx_user` (
    `id` int primary key,
    `name` varchar(32) not null unique, -- '登录名',
    `email` varchar(32), -- '电子邮箱',
    `password` varchar(32) not null, -- '密码',
    `true_name` varchar(16), -- '真实姓名',
    `department` int not null, -- '部门编号',
    `icon` text, -- 'base64图片',
    `status` int(1) not null default 1, -- '1可用，0停用',
    `gmt_create` datetime not null, -- '创建时间',
    `gmt_modify` datetime not null, -- '修改时间'
);

drop table if exists `fx_department`;
create table `fx_department` (
    `id` int primary key,
    `parent_id` int not null default 0, -- '上级部门id，0为一级部门',
    `name` varchar(32) not null, -- '部门名称',
    `gmt_create` datetime not null, -- '创建时间',
    `gmt_modify` datetime not null -- '修改时间'
);

drop table if exists `fx_chatlog`;
create table `fx_chatlog` (
    `id` int primary key,
    `sourceid` int not null, -- '当前用户id',
    `targetid` int not null, -- '目标用户id',
    `type` int(1) not null, -- '消息方向，1发送，2接收',
    `msg` text, -- '消息主体',
    `gmt_create` datetime not null -- '创建时间'
); -- '聊天记录表';
