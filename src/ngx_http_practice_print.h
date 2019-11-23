#include <ngx_core.h>
#include <ngx_http.h>


//print 命令的 set函数
char *ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// print 的 handler
ngx_int_t ngx_http_print_handler(ngx_http_request_t *r);