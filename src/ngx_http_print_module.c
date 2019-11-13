#define DDEBUG 1

#include "ddebug.h"
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
    conf->print.len = 0;
    conf->print.data = NULL;
    return conf;
}

static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child){
    ngx_http_print_loc_conf_t *prev = parent;
    ngx_http_print_loc_conf_t *conf = child;

    if( prev->print.data != NULL ){
        conf->print.len = prev->print.len;
        conf->print.data = prev->print.data;
    }
    return NGX_CONF_OK;
}


static ngx_command_t ngx_http_print_commands[] = {
        {
                ngx_string("print"),
                NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
                ngx_http_print,
                NGX_HTTP_LOC_CONF_OFFSET,
                0,
                NULL
        },
        ngx_null_command
};


ngx_module_t ngx_http_print_module = {
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
    config = ngx_http_get_module_loc_conf(r, ngx_http_print_module);

    printf("读取到的config配置；%s\n",config->print.data); //这里读取到的是null;

    /*设置 content-type ,content-type 应该为 ngx_str_t 类型*/
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    ngx_buf_t *buf; //buf
    ngx_chain_t chain[1]; //初始化一个chain
    buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    chain[0].buf = buf;
    chain[0].next = NULL;
    buf->pos = config->print.data;
    buf->last = buf->pos + config->print.len;
    buf->memory = 1;
    buf->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = config->print.len;

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
    ngx_str_t *values =cf->args->elts;
    mycf->print = values[1];

    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_print_handler;
    return NGX_CONF_OK;
}
