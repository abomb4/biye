-- SQLite3 db
create table `fx_user` (
    `id` integer primary key,
    `name` varchar(32) not null unique, -- '登录名',
    `email` varchar(32), -- '电子邮箱',
    `password` varchar(32) not null, -- '密码',
    `true_name` varchar(16), -- '真实姓名',
    `department` integer not null, -- '部门编号',
    `icon` text, -- 'base64图片',
    `status` int(1) not null default 1, -- '1可用，0停用',
    `gmt_create` datetime not null, -- '创建时间',
    `gmt_modify` datetime not null, -- '修改时间'
);

create table `fx_department` (
    `id` integer primary key,
    `parent_id` integer not null default 0, -- '上级部门id，0为一级部门',
    `name` varchar(32) not null, -- '部门名称',
    `gmt_create` datetime not null, -- '创建时间',
    `gmt_modify` datetime not null -- '修改时间'
);

create table `fx_chatlog` (
    `id` integer primary key,
    `sourceid` integer not null, -- '当前用户id',
    `targetid` integer not null, -- '目标用户id',
    `type` int(1) not null, -- '消息方向，1发送，2接收',
    `msg` text, -- '消息主体',
    `gmt_create` datetime not null -- '创建时间'
); -- '聊天记录表';

create table `fx_recent_contact` (
    `userid` integer not null, -- 用户id
    `contactid` integer not null, -- 联系人id
    `last_time` datetime not null, -- 最后联系时间
    constraint pk_recent primary key (userid, contactid)
); -- 最近联系人

create table `status` (
    `key` varchar(64) not null primary key,
    `value` varchar(255) not null default ''
); -- 系统状态表

insert into `status` values
('last_user_update_time', ''), -- 用户列表更新时间
('last_department_update_time', ''), -- 部门信息更新时间
('last_user_name', '') -- 上次登录用户名
;
