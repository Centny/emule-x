/*==============================================================*/
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2017/1/19 18:49:29                           */
/*==============================================================*/


drop index es_f_status on ex_file;

drop index es_f_album on ex_file;

drop index es_f_desc on ex_file;

drop index es_f_authors on ex_file;

drop index es_f_codec on ex_file;

drop index es_f_bitrate on ex_file;

drop index es_f_duration on ex_file;

drop index es_f_location on ex_file;

drop index es_f_format on ex_file;

drop index es_f_size on ex_file;

drop index es_f_filename on ex_file;

drop index es_f_emd4 on ex_file;

drop index es_f_md5 on ex_file;

drop index es_f_sha on ex_file;

drop table if exists ex_file;

drop table if exists ex_task;

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
/* Index: es_f_sha                                              */
/*==============================================================*/
create unique index es_f_sha on ex_file
(
   sha
);

/*==============================================================*/
/* Index: es_f_md5                                              */
/*==============================================================*/
create unique index es_f_md5 on ex_file
(
   md5
);

/*==============================================================*/
/* Index: es_f_emd4                                             */
/*==============================================================*/
create unique index es_f_emd4 on ex_file
(
   emd4
);

/*==============================================================*/
/* Index: es_f_filename                                         */
/*==============================================================*/
create index es_f_filename on ex_file
(
   filename
);

/*==============================================================*/
/* Index: es_f_size                                             */
/*==============================================================*/
create index es_f_size on ex_file
(
   size
);

/*==============================================================*/
/* Index: es_f_format                                           */
/*==============================================================*/
create index es_f_format on ex_file
(
   format
);

/*==============================================================*/
/* Index: es_f_location                                         */
/*==============================================================*/
create index es_f_location on ex_file
(
   location
);

/*==============================================================*/
/* Index: es_f_duration                                         */
/*==============================================================*/
create index es_f_duration on ex_file
(
   duration
);

/*==============================================================*/
/* Index: es_f_bitrate                                          */
/*==============================================================*/
create index es_f_bitrate on ex_file
(
   bitrate
);

/*==============================================================*/
/* Index: es_f_codec                                            */
/*==============================================================*/
create index es_f_codec on ex_file
(
   codec
);

/*==============================================================*/
/* Index: es_f_authors                                          */
/*==============================================================*/
create index es_f_authors on ex_file
(
   authors
);

/*==============================================================*/
/* Index: es_f_desc                                             */
/*==============================================================*/
create index es_f_desc on ex_file
(
   description
);

/*==============================================================*/
/* Index: es_f_album                                            */
/*==============================================================*/
create index es_f_album on ex_file
(
   album
);

/*==============================================================*/
/* Index: es_f_status                                           */
/*==============================================================*/
create index es_f_status on ex_file
(
   status
);

/*==============================================================*/
/* Table: ex_task                                               */
/*==============================================================*/
create table ex_task
(
   filename             varchar(256) binary not null,
   location             varchar(4096) binary,
   size                 int not null,
   ext                  varchar(8)
);

