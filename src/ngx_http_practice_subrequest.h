//上下文
typedef struct {
    ngx_str_t res;
} ngx_http_sub_ctx_t;

//upstream 命令的 set函数
char *ngx_http_sub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// upstream 的 handler
ngx_int_t ngx_http_sub_handler(ngx_http_request_t *r);


static ngx_int_t subrequst_post_handler(ngx_http_request_t *r, void *data, ngx_int_t  rc);

//父请求的回调
static void sub_post_handler(ngx_http_request_t *r);
