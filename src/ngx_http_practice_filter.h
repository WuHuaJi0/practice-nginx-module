#include <ngx_core.h>
#include <ngx_http.h>

ngx_int_t ngx_http_filter_init(ngx_conf_t *cf);

//print 命令的 set函数
char *ngx_http_filter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

ngx_int_t ngx_http_sign_body_filter(ngx_http_request_t *r, ngx_chain_t *in);

ngx_int_t ngx_http_sign_header_filter(ngx_http_request_t *r);
