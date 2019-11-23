//
// Created by 吴化吉 on 11/19/19.
//
#include "ngx_http_practice_module.h"
#include "ngx_http_practice_print.h"
#include "ngx_http_practice_upstream.h"


static void *ngx_create_loc_conf(ngx_conf_t *cf ){
    ngx_http_practice_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_practice_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->print_args.len = 0;
    conf->print_args.data = NULL;

    //先临时硬编码 practice_upstream 的配置；
    //超时设置
    conf->upstream_conf.connect_timeout = 60000;
    conf->upstream_conf.read_timeout = 60000;
    conf->upstream_conf.send_timeout = 60000;
    conf->upstream_conf.store_timeout = 60000;
    conf->upstream_conf.buffering = 0;
    conf->upstream_conf.bufs.num = 8;
    conf->upstream_conf.bufs.size = ngx_pagesize;
    conf->upstream_conf.buffer_size = ngx_pagesize;
    conf->upstream_conf.busy_buffer_size = 2 * ngx_pagesize;
    conf->upstream_conf.temp_file_write_size = 2 * ngx_pagesize;
    conf->upstream_conf.max_temp_file_size = 1024 * 1024 * 1024;
    conf->upstream_conf.hide_headers = NGX_CONF_USET_PTR;
    conf->upstream_conf.pass_headers = NGX_CONF_USET_PTR;

    return conf;
}

static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child){
    ngx_http_practice_loc_conf_t *prev = parent;
    ngx_http_practice_loc_conf_t *conf = child;

    if( conf->print_args.data == NULL ){
        conf->print_args.len = prev->print_args.len;
        conf->print_args.data = prev->print_args.data;
    }

    return NGX_CONF_OK;
}

static ngx_command_t ngx_http_practice_commands[] = {
    {
        ngx_string("print"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
        ngx_http_print,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
        ngx_string("practice_upstream"),
        NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
        ngx_http_practice_upstream,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

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

//模块定义
ngx_module_t ngx_http_practice_module = {
        NGX_MODULE_V1,
        &ngx_http_print_module_ctx,
        ngx_http_practice_commands,
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
