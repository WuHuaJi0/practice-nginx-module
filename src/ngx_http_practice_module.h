//
// Created by 吴化吉 on 11/19/19.
//
#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>

extern ngx_module_t ngx_http_practice_module;

typedef enum {
    practice_print = 0,
    practice_filter,
    practice_upstream,
    practice_subrequest,
} ngx_http_practice_command_type_t; //指令类型

/* location config struct */
typedef struct {
    ngx_http_practice_command_type_t type;
    ngx_str_t print_args; // print的参数
    ngx_int_t enable_sign; //on : 1 / off: 0
    ngx_http_upstream_conf_t upstream_conf;
} ngx_http_practice_loc_conf_t;


typedef struct {
    ngx_flag_t enable;
} ngx_http_practice_tmp_conf_t;

//请求上下文
typedef struct {
    ngx_http_status_t status;
    ngx_str_t backendServer;
} ngx_http_practice_ctx_t;

void *ngx_create_loc_conf(ngx_conf_t *cf);

char *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

