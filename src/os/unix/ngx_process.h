
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_H_INCLUDED_
#define _NGX_PROCESS_H_INCLUDED_


#include <ngx_setaffinity.h>
#include <ngx_setproctitle.h>


typedef pid_t       ngx_pid_t;

#define NGX_INVALID_PID  -1

typedef void (*ngx_spawn_proc_pt) (ngx_cycle_t *cycle, void *data);

/*
 * Nginx关于进程的封装
 */
typedef struct {
    ngx_pid_t           pid;///<　当前工作进程的 pid号
    int                 status;///<　当前进程的退出状态
    ngx_socket_t        channel[2];///< 用于进程间通信的 Socket句柄

    ngx_spawn_proc_pt   proc;///<　指向工作进程执行的函数
    void               *data;
    char               *name;

    unsigned            respawn:1;///<　为1时表示重新创建
    unsigned            just_spawn:1;///<　为1时表示首次创建
    unsigned            detached:1;///<　为1时表示以分离
    unsigned            exiting:1;///<　为1时表示正在退出
    unsigned            exited:1;///<　为1时表示已退出
} ngx_process_t;


typedef struct {
    char         *path;
    char         *name;
    char *const  *argv;
    char *const  *envp;
} ngx_exec_ctx_t;

// Nginx最大进程数
#define NGX_MAX_PROCESSES         1024

#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2
#define NGX_PROCESS_RESPAWN       -3
#define NGX_PROCESS_JUST_RESPAWN  -4
#define NGX_PROCESS_DETACHED      -5


#define ngx_getpid   getpid

#ifndef ngx_log_pid
#define ngx_log_pid  ngx_pid
#endif

//产生 worker进程的重要函数
ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle,
    ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn);
ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx);
//初始化所有的信号处理函数
ngx_int_t ngx_init_signals(ngx_log_t *log);
void ngx_debug_point(void);


#if (NGX_HAVE_SCHED_YIELD)
#define ngx_sched_yield()  sched_yield()
#else
#define ngx_sched_yield()  usleep(1)
#endif


extern int            ngx_argc;
extern char         **ngx_argv;
extern char         **ngx_os_argv;

extern ngx_pid_t      ngx_pid;
extern ngx_socket_t   ngx_channel;
extern ngx_int_t      ngx_process_slot;
extern ngx_int_t      ngx_last_process;
//这个数组表示了　Nginx当前所有的进程
extern ngx_process_t  ngx_processes[NGX_MAX_PROCESSES];


#endif /* _NGX_PROCESS_H_INCLUDED_ */
