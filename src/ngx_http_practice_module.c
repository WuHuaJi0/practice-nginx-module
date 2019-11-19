//
// Created by 吴化吉 on 11/19/19.
//
#include "ngx_http_practice_module.h"
#include "ngx_http_practice_print.h"


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

static ngx_command_t ngx_http_practice_commands[] = {
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
