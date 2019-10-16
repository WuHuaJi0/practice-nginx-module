#define DDEBUG 1

#include "ddebug.h"
#include "ngx_http_print_module.h"
#include <ngx_config.h>

static char *ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_print_handler(ngx_http_request_t *r);


static char *
ngx_http_print(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    /*todo: 这里的conf是从哪传来的？从context的 create_conf中传来的*/
    ngx_http_core_loc_conf_t *clcf; //找到print所属的配置块，
    /*这里一个宏，来找到conf*/
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_print_handler;
    return NGX_CONF_OK;
}


static ngx_command_t ngx_http_print_commands[] = {
        {
                ngx_string("print"),
                NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF
                | NGX_CONF_NOARGS,
                ngx_http_print,
                NGX_HTTP_LOC_CONF_OFFSET,
                0,
                NULL
        },
        ngx_null_command
};

static ngx_http_module_t ngx_http_print_module_ctx = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
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
    ngx_int_t rsh; //result send header,
    ngx_buf_t *b; //buf

    ngx_chain_t out[1];
    ngx_http_print_loc_conf_t *config;
    config = ngx_http_get_module_loc_conf(r, ngx_http_print_module);

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    out[0].buf = b;
    out[0].next = NULL;
    b->pos = config->print.data;
    b->last = b->pos + config->print.len;
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = config->print.len;

    rsh = ngx_http_send_header(r);
    if (rsh == NGX_ERROR || rsh > NGX_OK || r->header_only) {
        return rsh;
    }
    int result = ngx_http_output_filter(r, &out[0]);
    return result;
}

