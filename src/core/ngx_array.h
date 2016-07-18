/*
 *此文件是Nginx自行封装的数组，为了适应不同的数据类型而用法与一般的数组有些区别。
 */
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*Nginx关于数组的结构体定义
 *
 */
typedef struct {
    void        *elts;  //指向实际的数据存储区域
    ngx_uint_t   nelts; //数组实际元素的个数
    size_t       size;  //数组单个元素的大小
    ngx_uint_t   nalloc;//数组的存储容量，当nelts大于此至时，引起数组的扩容
    ngx_pool_t  *pool;  //用来分配内存的内存池
} ngx_array_t;

//创建一个数组对象
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);

//摧毁数组对象
void ngx_array_destroy(ngx_array_t *a);

/*向数组追加一个元素
 *注意：Nginx数组的用法是使用此函数增加一个元素后返回该元素的地址，需要自行强制转换然后赋值
 */
void *ngx_array_push(ngx_array_t *a);

/*向数组追加 n个元素，返回新增加首元素的地址
 *同样需要向上个函数一样进行强制转换
 */
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);

/*初始化一个数组*/
static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
