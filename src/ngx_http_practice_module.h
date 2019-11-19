//
// Created by 吴化吉 on 11/19/19.
//
#include <ngx_core.h>
#include <ngx_config.h>

extern ngx_module_t  ngx_http_practice_module;


static void *ngx_create_loc_conf(ngx_conf_t *cf);
static void *ngx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

