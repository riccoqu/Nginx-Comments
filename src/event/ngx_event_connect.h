
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_CONNECT_H_INCLUDED_
#define _NGX_EVENT_CONNECT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_PEER_KEEPALIVE           1
#define NGX_PEER_NEXT                2
#define NGX_PEER_FAILED              4


typedef struct ngx_peer_connection_s  ngx_peer_connection_t;

typedef ngx_int_t (*ngx_event_get_peer_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_free_peer_pt)(ngx_peer_connection_t *pc, void *data,
    ngx_uint_t state);
#if (NGX_SSL)

typedef ngx_int_t (*ngx_event_set_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_save_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
#endif

/*
 * ngx_peer_connection_s结构体表示主动连接,用在例如 upstream时会用到
 */

struct ngx_peer_connection_s {
    ngx_connection_t                *connection;///< 因为需要重用,所以包含了 ngx_connection_t

    struct sockaddr                 *sockaddr;///< 远端服务器的 socket地址
    socklen_t                        socklen;///< sockaddr地址长度
    ngx_str_t                       *name;///< 远端服务器名称

    ngx_uint_t                       tries;///< 连接出现失败时可以重试的次数
    ngx_msec_t                       start_time;///< 连接开始的时间

    ngx_event_get_peer_pt            get;///< 获取连接的方法
    ngx_event_free_peer_pt           free;///< 释放连接的方法
    void                            *data;///< 用于与 get和 free配合传递参数

#if (NGX_SSL)
    ngx_event_set_peer_session_pt    set_session;///< SSL的 get方法
    ngx_event_save_peer_session_pt   save_session;///< SSL的 free方法
#endif

    ngx_addr_t                      *local;///< 本机地址信息

    int                              type;///< 连接的类型
    int                              rcvbuf;///< 套接字接收缓冲区的大小

    ngx_log_t                       *log;///< 记录日志的对象

    unsigned                         cached:1;///< 为1时表示上面的 connection连接已缓存
#if (NGX_HAVE_TRANSPARENT_PROXY)
    unsigned                         transparent:1;
#endif

                                     /* ngx_connection_log_error_e */
    unsigned                         log_error:2;///< 错误的标志位
};


ngx_int_t ngx_event_connect_peer(ngx_peer_connection_t *pc);
ngx_int_t ngx_event_get_peer(ngx_peer_connection_t *pc, void *data);


#endif /* _NGX_EVENT_CONNECT_H_INCLUDED_ */
