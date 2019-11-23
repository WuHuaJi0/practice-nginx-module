//
// Created by 吴化吉 on 11/19/19.
//
#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
//#include <ngx_http_upstream.h>

extern ngx_module_t  ngx_http_practice_module;

/* location config struct */
typedef struct {
    ngx_str_t print_args; //print的参数
    ngx_http_upstream_conf_t upstream_conf;
} ngx_http_practice_loc_conf_t;

//请求上下文
typedef struct {
   ngx_http_status_t status;
} ngx_http_practice_ctx_t;

static void *ngx_create_loc_conf(ngx_conf_t *cf);
static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

