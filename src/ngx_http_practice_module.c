//
// Created by 吴化吉 on 11/19/19.
//
#include "ngx_http_practice_module.h"
#include "ngx_http_practice_print.h"
#include "ngx_http_practice_filter.h"
//#include "ngx_http_practice_upstream.h"
//#include "ngx_http_practice_subrequest.h"


//static ngx_str_t  ngx_http_proxy_hide_headers[] =
//{
//        ngx_string("Date"),
//        ngx_string("Server"),
//        ngx_string("X-Pad"),
//        ngx_string("X-Accel-Expires"),
//        ngx_string("X-Accel-Redirect"),
//        ngx_string("X-Accel-Limit-Rate"),
//        ngx_string("X-Accel-Buffering"),
//        ngx_string("X-Accel-Charset"),
//        ngx_null_string
//};


void *ngx_create_loc_conf(ngx_conf_t *cf ){
    ngx_http_practice_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_practice_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->type = NGX_CONF_UNSET;

    conf->print_args.len = 0;
    conf->print_args.data = NULL;

    //先临时硬编码 practice_upstream 的配置；
    //超时设置
//    conf->upstream_conf.connect_timeout = 6000;
//    conf->upstream_conf.read_timeout = 6000;
//    conf->upstream_conf.send_timeout = 6000;
//    conf->upstream_conf.store_access = 0600;
//    conf->upstream_conf.buffering = 0;
//    conf->upstream_conf.bufs.num = 8;
//    conf->upstream_conf.bufs.size = ngx_pagesize;
//    conf->upstream_conf.buffer_size = ngx_pagesize;
//    conf->upstream_conf.busy_buffers_size = 2 * ngx_pagesize;
//    conf->upstream_conf.temp_file_write_size = 2 * ngx_pagesize;
//    conf->upstream_conf.max_temp_file_size = 1024 * 1024 * 1024;
//    conf->upstream_conf.hide_headers = NGX_CONF_UNSET_PTR;
//    conf->upstream_conf.pass_headers = NGX_CONF_UNSET_PTR;


    return conf;
}

char* ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child){
//    ngx_http_practice_loc_conf_t *prev = (ngx_http_practice_loc_conf_t *)parent;
//    ngx_http_practice_loc_conf_t *conf = (ngx_http_practice_loc_conf_t *)child;

//    ngx_hash_init_t             hash;
//    hash.max_size = 100;
//    hash.bucket_size = 1024;
//    hash.name = "proxy_headers_hash";
//    if (ngx_http_upstream_hide_headers_hash(cf, &conf->upstream_conf,
//                                            &prev->upstream_conf, ngx_http_proxy_hide_headers, &hash)
//        != NGX_OK)
//    {
//        return NGX_CONF_ERROR;
//    }

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
        ngx_string("add_content"), //向响应头和响应体添加内容（sign）
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
        ngx_http_filter,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
//        ngx_string("practice_upstream"),
//        NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
//        ngx_http_upstream,
//        NGX_HTTP_LOC_CONF_OFFSET,
//        0,
//        NULL
    },
    {
//        ngx_string("sub"),
//        NGX_HTTP_LOC_CONF | NGX_CONF_ANY,
//        ngx_http_sub,
//        NGX_HTTP_LOC_CONF_OFFSET,
//        0,
//        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_print_module_ctx = {
        NULL,/* preconfiguration */
        ngx_http_filter_init,/* postconfiguration */
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
