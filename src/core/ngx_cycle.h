
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};

/**
 * 关于ngx_cycle_s结构体可以参考:
 * @see http://blog.csdn.net/livelylittlefish/article/details/7247080
 *
 * Ngxin的初始化围绕此结构体进行
 */
struct ngx_cycle_s {
    void                  ****conf_ctx; ///< 所有模块的配置上下文数组
    ngx_pool_t               *pool;	    ///< 内存池

    ngx_log_t                *log;	    ///< 日志
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  ///< unsigned  log_use_stderr:1;

    ngx_connection_t        **files;	///< 连接文件
    /*
     * 这里由 ngx_connection_t组成的链表中,由 ngx_connection_t.data指针保存下个节点的地址
     */
    ngx_connection_t         *free_connections; ///< 空闲连接
    ngx_uint_t                free_connection_n;///< 空闲连接的个数

    ngx_module_t            **modules;	///< 模块数组
    ngx_uint_t                modules_n;///< 模块数组的个数
    ngx_uint_t                modules_used;///< unsigned  modules_used:1;

    ngx_queue_t               reusable_connections_queue;///< 再连接队列

    ngx_array_t               listening;  ///< 监听套接字的数组
    ngx_array_t               paths;      ///< 路径的数组
    ngx_array_t               config_dump;
    ngx_list_t                open_files;	///< 打开文件的链表
    ngx_list_t                shared_memory;	///< 共享内存的链表

    ngx_uint_t                connection_n;	///< 连接个数
    ngx_uint_t                files_n;		///< 打开文件个数
    /*
     *  ngx_cycle_t结构体中 connections数组、read_events数组和 write_events数组以
     *  相同的数组序号进行连接,相同序号下这三个数组中的元素是配合使用的
     */
    ngx_connection_t         *connections;	///< 链接
    ngx_event_t              *read_events;	///< 读事件
    ngx_event_t              *write_events;	///< 写事件

    ngx_cycle_t              *old_cycle;    ///< 旧的循环

    ngx_str_t                 conf_file;    ///< 配置文件名
    ngx_str_t                 conf_param;   ///< 配置参数
    ngx_str_t                 conf_prefix;  ///< 配置前缀
    ngx_str_t                 prefix;		    ///< 前缀
    ngx_str_t                 lock_file;	  ///< 锁文件
    ngx_str_t                 hostname;		  ///< 主机名
};


typedef struct {
    ngx_flag_t                daemon;///< 为1时启用守护进程
    ngx_flag_t                master;

    ngx_msec_t                timer_resolution;

    ngx_int_t                 worker_processes;///<　work进程数
    ngx_int_t                 debug_points;

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;///<　运行时的用户名
    ngx_uid_t                 user;///<　运行时的 uid
    ngx_gid_t                 group;///<　运行时的 gid

    ngx_str_t                 working_directory;///<　工作目录
    ngx_str_t                 lock_file;///<　锁文件

    ngx_str_t                 pid;///<　pid的字符表示
    ngx_str_t                 oldpid;

    ngx_array_t               env;
    char                    **environment;
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)

/**
  *用于初始化循环的函数
  */
ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
//根据启动参数和记录pid的文件,向 master进程发送相应的信号
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
/*
 *　执行新二进制程序的函数,可用于热升级
 */
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
