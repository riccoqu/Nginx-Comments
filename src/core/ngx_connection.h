
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text;

    int                 type;

    int                 backlog;
    int                 rcvbuf;
    int                 sndbuf;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler;

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;
    ngx_connection_t   *connection;

    ngx_uint_t          worker;

    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_HTTP_V2_BUFFERED   0x02

/*
 * 使用 ngx_connection_t表示连接
 */
struct ngx_connection_s {
    /*
     * 连接未使用时, data成员用于充当连接池中空闲连接链表中的 next指针
     * 当链接被使用时, data成员的意义由使用它的模块自定义
     * 例如: HTTP模块中, data指向 ngx_http_request_t请求
     */
    void               *data;
    ngx_event_t        *read;///< 连接对应的读事件
    ngx_event_t        *write;///< 连接对应的写事件

    ngx_socket_t        fd;///< Socket的文件描述符

    /*
     *   下面这四个回调方法都由不同的事件消费模块决定其行为
     */

    ngx_recv_pt         recv;///< 直接接收网络字节流的方法
    ngx_send_pt         send;///< 直接发送网络字节流的方法
    ngx_recv_chain_pt   recv_chain;///< ngx_chain_t为参数的接收网络字节流的方法
    ngx_send_chain_pt   send_chain;///< ngx_chain_t为参数的发送网络字节流的方法

    ngx_listening_t    *listening;///< 每个连接对应的 ngx_listening_t对象

    off_t               sent;///< 当前连接上已经发送的字节数

    ngx_log_t          *log;///< 可以记录日志的 ngx_lot_t对象

    ngx_pool_t         *pool;///< 内存池

    int                 type;

    struct sockaddr    *sockaddr;///< 连接客户端的 sockaddr结构体
    socklen_t           socklen;///< 结构体的长度
    ngx_str_t           addr_text;///< 连接客户端字符串形式的 IP地址

    ngx_str_t           proxy_protocol_addr;///<
    in_port_t           proxy_protocol_port;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;///< 本机的监听端口对应的 sockaddr结构体
    socklen_t           local_socklen;///< sockaddr结构体的大小
    /*
     * 用于接受客户端发来的字节流,缓冲区的大小由不同的事件模块自行分配
     */
    ngx_buf_t          *buffer;///< 缓冲区
    /*
     * 将当前连接串联为一个链表,添加到 ngx_cycle_t的
     * reusable_connections_qyeye双向连表中,表示可以重用的连接
     */
    ngx_queue_t         queue;///< 关于 ngx_queue_t的用法见 core/ngx_queue.h

    ngx_atomic_uint_t   number;///< 连接使用次数,每次使用时都会 +1

    ngx_uint_t          requests;///< 处理的请求次数

    /*
     * 以下都为标志位
     */

    unsigned            buffered:8;///< 缓存中的业务类型

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            timedout:1;///< 为1时表示连接已超时
    unsigned            error:1;///< 为1表示处理过程中出现错误
    unsigned            destroyed:1;///< 为1时表示当前 TCP连接已经被销毁

    unsigned            idle:1;///< 为1时表示连接处于空闲状态
    unsigned            reusable:1;///< 为1时表示连接可重用
    unsigned            close:1;///< 为1时表示连接关闭
    unsigned            shared:1;///< 为1时表示当前连接共享的

    unsigned            sendfile:1;///< 为1时表示正在将文件中的数据发往连接的另一端
    unsigned            sndlowat:1;///< 为1时表示缓冲区满足阈值时才会分发事件
    /*
     * 表示如何使用 TCP的 NoDelay特性
     */
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    /*
     * 表示如何使用 TCP的 NoPush特性
     */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, struct sockaddr *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_clone_listening(ngx_conf_t *cf, ngx_listening_t *ls);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
void ngx_close_idle_connections(ngx_cycle_t *cycle);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
