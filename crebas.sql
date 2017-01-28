/*==============================================================*/
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2017/1/23 15:38:32                           */
/*==============================================================*/


drop table if exists ex_env;

drop index ex_f_status on ex_file;

drop index ex_f_album on ex_file;

drop index ex_f_desc on ex_file;

drop index ex_f_authors on ex_file;

drop index ex_f_codec on ex_file;

drop index ex_f_bitrate on ex_file;

drop index ex_f_duration on ex_file;

drop index ex_f_location on ex_file;

drop index ex_f_format on ex_file;

drop index ex_f_size on ex_file;

drop index ex_f_filename on ex_file;

drop index ex_f_emd4 on ex_file;

drop index ex_f_md5 on ex_file;

drop index ex_f_sha on ex_file;

drop table if exists ex_file;

drop index ex_t_status on ex_task;

drop index ex_t_used on ex_task;

drop index ex_t_ext on ex_task;

drop index ex_t_done on ex_task;

drop index ex_t_size on ex_task;

drop index ex_t_location on ex_task;

drop index ex_t_filename on ex_task;

drop table if exists ex_task;

/*==============================================================*/
/* Table: ex_env                                                */
/*==============================================================*/
create table ex_env
(
   name                 varchar(32) not null,
   vala                 varchar(10240),
   valb                 varchar(10240),
   valc                 varchar(10240),
   vald                 varchar(10240),
   primary key (name)
);

/*==============================================================*/
/* Table: ex_file                                               */
/*==============================================================*/
create table ex_file
(
   sha                  blob,
   md5                  blob,
   emd4                 blob,
   filename             varchar(256) not null,
   size                 int not null,
   format               varchar(16),
   location             varchar(10240),
   duration             numeric,
   bitrate              numeric,
   codec                varchar(32),
   authors              varchar(256),
   description          varchar(2048),
   album                varchar(256),
   status               int not null
);

/*==============================================================*/
/* Index: ex_f_sha                                              */
/*==============================================================*/
create unique index ex_f_sha on ex_file
(
   sha
);

/*==============================================================*/
/* Index: ex_f_md5                                              */
/*==============================================================*/
create unique index ex_f_md5 on ex_file
(
   md5
);

/*==============================================================*/
/* Index: ex_f_emd4                                             */
/*==============================================================*/
create unique index ex_f_emd4 on ex_file
(
   emd4
);

/*==============================================================*/
/* Index: ex_f_filename                                         */
/*==============================================================*/
create index ex_f_filename on ex_file
(
   filename
);

/*==============================================================*/
/* Index: ex_f_size                                             */
/*==============================================================*/
create index ex_f_size on ex_file
(
   size
);

/*==============================================================*/
/* Index: ex_f_format                                           */
/*==============================================================*/
create index ex_f_format on ex_file
(
   format
);

/*==============================================================*/
/* Index: ex_f_location                                         */
/*==============================================================*/
create index ex_f_location on ex_file
(
   location
);

/*==============================================================*/
/* Index: ex_f_duration                                         */
/*==============================================================*/
create index ex_f_duration on ex_file
(
   duration
);

/*==============================================================*/
/* Index: ex_f_bitrate                                          */
/*==============================================================*/
create index ex_f_bitrate on ex_file
(
   bitrate
);

/*==============================================================*/
/* Index: ex_f_codec                                            */
/*==============================================================*/
create index ex_f_codec on ex_file
(
   codec
);

/*==============================================================*/
/* Index: ex_f_authors                                          */
/*==============================================================*/
create index ex_f_authors on ex_file
(
   authors
);

/*==============================================================*/
/* Index: ex_f_desc                                             */
/*==============================================================*/
create index ex_f_desc on ex_file
(
   description
);

/*==============================================================*/
/* Index: ex_f_album                                            */
/*==============================================================*/
create index ex_f_album on ex_file
(
   album
);

/*==============================================================*/
/* Index: ex_f_status                                           */
/*==============================================================*/
create index ex_f_status on ex_file
(
   status
);

/*==============================================================*/
/* Table: ex_task                                               */
/*==============================================================*/
create table ex_task
(
   tid                  int not null,
   filename             varchar(256) not null,
   location             varchar(4096) not null,
   size                 int not null,
   done                 int not null,
   format               varchar(8),
   used                 int not null,
   status               int not null,
   primary key (tid)
);

/*==============================================================*/
/* Index: ex_t_filename                                         */
/*==============================================================*/
create index ex_t_filename on ex_task
(
   filename
);

/*==============================================================*/
/* Index: ex_t_location                                         */
/*==============================================================*/
create index ex_t_location on ex_task
(
   location
);

/*==============================================================*/
/* Index: ex_t_size                                             */
/*==============================================================*/
create index ex_t_size on ex_task
(
   size
);

/*==============================================================*/
/* Index: ex_t_done                                             */
/*==============================================================*/
create index ex_t_done on ex_task
(
   done
);

/*==============================================================*/
/* Index: ex_t_ext                                              */
/*==============================================================*/
create index ex_t_ext on ex_task
(
   format
);

/*==============================================================*/
/* Index: ex_t_used                                             */
/*==============================================================*/
create index ex_t_used on ex_task
(
   used
);

/*==============================================================*/
/* Index: ex_t_status                                           */
/*==============================================================*/
create index ex_t_status on ex_task
(
   status
);

