
#include "ngx_http_print_module.h"
#include <ngx_config.h>

static char *ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_print_handler(ngx_http_request_t *r);
static void *ngx_create_loc_conf(ngx_conf_t *cf);
static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_http_module_t ngx_http_print_module_ctx = {
        NULL,/* preconfiguration */
        NULL,/* postconfiguration */
        NULL,/* create main configuration */
        NULL,/* init main configuration */
        NULL,
        NULL,/* merge server configuration */
        ngx_create_loc_conf,
        ngx_merge_loc_conf,
};

static void *ngx_create_loc_conf(ngx_conf_t *cf ){
    ngx_http_print_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_print_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->args.len = 0;
    conf->args.data = NULL;
    return conf;
}

static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child){
    ngx_http_print_loc_conf_t *prev = parent;
    ngx_http_print_loc_conf_t *conf = child;

    if( conf->args.data == NULL ){
        conf->args.len = prev->args.len;
        conf->args.data = prev->args.data;
    }
    return NGX_CONF_OK;
}


static ngx_command_t ngx_http_print_commands[] = {
        {
                ngx_string("print"),
                NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
                ngx_http_print,
                NGX_HTTP_LOC_CONF_OFFSET,
                0,
                NULL
        },
        ngx_null_command
};


ngx_module_t ngx_http_practice_module = {
        NGX_MODULE_V1,
        &ngx_http_print_module_ctx,
        ngx_http_print_commands,
        NGX_HTTP_MODULE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_print_handler(ngx_http_request_t *r) {
    //这里是做什么？ 读取到配置项吗？
    ngx_http_print_loc_conf_t *config;
    config = ngx_http_get_module_loc_conf(r, ngx_http_practice_module);


    /*设置 content-type ,content-type 应该为 ngx_str_t 类型*/
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    ngx_buf_t *buf; //buf
    ngx_chain_t chain[1]; //初始化一个chain
    buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    chain[0].buf = buf;
    chain[0].next = NULL;

    buf->pos = config->args.data;
    buf->last = buf->pos + config->args.len;

    buf->memory = 1;
    buf->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = config->args.len;

    ngx_int_t result_send_header = ngx_http_send_header(r);
    if (result_send_header == NGX_ERROR || result_send_header > NGX_OK || r->header_only) {
        return result_send_header;
    }

    int result = ngx_http_output_filter(r, &chain[0]);
    return result;
}

//set 函数；
static char *ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_print_loc_conf_t *mycf = conf;

    ngx_uint_t args_count = cf->args->nelts; //配置项参数个数，包含指令本身；
    ngx_str_t *values = cf->args->elts;

    if( args_count == 1  ){ // 如果没有参数
        ngx_str_t string = ngx_string("print without args");
        mycf->args = string;
    }else{
        int size_args = 0;
        for (int i = 1; i < args_count; ++i) {
            size_args += values[i].len;
        }
        size_args += args_count - 1; //留出空间给空格使用；
        u_char *string = ngx_pcalloc(cf->pool, size_args);

        u_char *des = string;
        for (int i = 1; i < args_count; ++i) {
            des = ngx_cpystrn(des ,values[i].data, values[i].len + 1); //todo:非常奇怪，这里需要len+1才能完整复制成功
            des = ngx_cpystrn(des ," ", 2);
        }
        mycf->args.data = string;
        mycf->args.len = size_args;
    }


    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_print_handler;
    return NGX_CONF_OK;
}
