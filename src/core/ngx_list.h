
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;
/**
 * Nginx中对于链表节点的封装
 * 链表节点表示一个数组,且数组中的元素不能大于 ngx_list_t.size的大小
 * 这样的设计使得 ngx_list_t 可以存储任意类型的结构
 * 但是可能会造成一部分空间的未使用
 */
struct ngx_list_part_s {
    void             *elts;/**< 数组的起始地址*/
    ngx_uint_t        nelts;/**< 表示数组中已有的元素个数*/
    ngx_list_part_t  *next;/**< 表示下一个节点*/
};
/**
 * Nginx中对于链表的封装
 * 注意,链表中的一个节点表示一个数组,既每个节点构成一个数组，而一个节点又是另一个数组
 * @see 《深入理解Nginx》 P73
 */
typedef struct {
    ngx_list_part_t  *last;	/**< 链表的最后一个数组元素*/
    ngx_list_part_t   part;	/**< 链表的首个数组元素*/
    size_t            size;	/**< 表示一个 ngx_list_part_s数组中一个数组元素的最大大小*/
    ngx_uint_t        nalloc;	/**< 表示一个 ngx_list_part_s 的容量*/
    ngx_pool_t       *pool;	/**< 内存池*/
} ngx_list_t;

/**
 * 创建一个链表
 */
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);
/**
 * 初始化链表
 */
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */

/**
 * 推入一个元素,返回该元素的地址,需要再通过类型转换赋值
 */
void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
