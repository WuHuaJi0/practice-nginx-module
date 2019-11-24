//
// Created by 吴化吉 on 11/23/19.
//

//upstream 命令的 set函数
char *ngx_http_practice_upstream(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


// upstream 的 handler
ngx_int_t ngx_http_practice_upstream_handler(ngx_http_request_t *r);


// upstream 的 create_request 方法；
static ngx_int_t practice_upstream_create_request(ngx_http_request_t *r) ;


//upstrem 的 reinit_request;
static ngx_int_t practice_upstream_reinit_request(ngx_http_request_t *r);

// abort
static ngx_int_t practice_upstream_abort_request(ngx_http_request_t *r);

//解析响应行
static ngx_int_t practice_process_status_line(ngx_http_request_t *r) ;


static ngx_int_t practice_upstream_process_header(ngx_http_request_t *r);


static void practice_upstream_finialize_request(ngx_http_request_t *r,ngx_int_t rc);
