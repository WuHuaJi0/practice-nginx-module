#include "ngx_http_practice_module.h"
#include "ngx_http_practice_print.h"

ngx_int_t ngx_http_print_handler(ngx_http_request_t *r) {

    //这里是做什么？ 读取到配置项吗？
    ngx_http_practice_loc_conf_t *config;
    config = ngx_http_get_module_loc_conf(r, ngx_http_practice_module);

    ngx_str_t content_type = ngx_string("text/plain");
    r->headers_out.content_type = content_type;

    ngx_buf_t *buf; //buf
    ngx_chain_t chain[1]; //初始化一个chain
    buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    chain[0].buf = buf;
    chain[0].next = NULL;

    buf->pos = config->print_args.data;
    buf->last = buf->pos + config->print_args.len;

    buf->memory = 1;
    buf->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = config->print_args.len;

    ngx_int_t result_send_header = ngx_http_send_header(r);
    if (result_send_header == NGX_ERROR || result_send_header > NGX_OK || r->header_only) {
        return result_send_header;
    }

    int result = ngx_http_output_filter(r, &chain[0]);
    return result;
}

//set 函数；
char *ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_practice_loc_conf_t *mycf = conf;

    ngx_int_t args_count = cf->args->nelts; //配置项参数个数，包含指令本身；
    ngx_str_t *values = cf->args->elts;

    if( args_count == 1  ){ // 如果没有参数
        ngx_str_t string = ngx_string("print without args");
        mycf->print_args = string;
    }else{
        int size_args = 0;
        for (ngx_int_t i = 1; i < args_count; ++i) {
            size_args += values[i].len;
        }
        size_args += args_count - 1; //留出空间给空格使用；
        u_char *string = ngx_pcalloc(cf->pool, size_args);

        u_char *des = string;
        for (ngx_int_t i = 1; i < args_count; ++i) {
            des = ngx_cpystrn(des ,values[i].data, values[i].len + 1); //todo:非常奇怪，这里需要len+1才能完整复制成功
            u_char space = ' ';
            des = ngx_cpystrn(des ,&space, 2);
        }
        mycf->print_args.data = string;
        mycf->print_args.len = size_args;
    }


    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_print_handler;
    return NGX_CONF_OK;
}
