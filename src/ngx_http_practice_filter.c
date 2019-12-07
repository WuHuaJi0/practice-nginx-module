#include "ngx_http_practice_module.h"
#include "ngx_http_practice_filter.h"

ngx_http_output_header_filter_pt ngx_http_next_header_filter;
ngx_http_output_body_filter_pt ngx_http_next_body_filter;

ngx_int_t ngx_http_filter_init(ngx_conf_t *cf)
{
    //插入到头部处理方法链表的首部
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_sign_header_filter;

    //插入到包体处理方法链表的首部
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_sign_body_filter;

    return NGX_OK;
}

//set 函数；
char *ngx_http_filter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_practice_loc_conf_t *mycf = conf;

    ngx_str_t *values = cf->args->elts;
    mycf->type = practice_filter;

    if( ngx_strncmp(values[1].data, "on", 2) == 0  ){
        mycf->enable_sign = 1;
    }else{
        mycf->enable_sign = 0;
    }

    return NGX_CONF_OK;
}


static ngx_str_t filter_prefix = ngx_string("[content add by filter]\n");

ngx_int_t ngx_http_sign_header_filter(ngx_http_request_t *r){

    ngx_http_practice_loc_conf_t *conf = ngx_http_get_module_loc_conf(r, ngx_http_practice_module);

    if( conf->enable_sign != 1 ){ //如果配置的是off，则不管；
        return ngx_http_next_header_filter(r);
    }

    //这里要修改 content-length;
    r->headers_out.content_length_n += filter_prefix.len;

    return ngx_http_next_header_filter(r);
}


ngx_int_t
ngx_http_sign_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_http_practice_loc_conf_t *conf = ngx_http_get_module_loc_conf(r, ngx_http_practice_module);

    if( conf->enable_sign != 1 ){ //如果配置的是off，则不管；
        return ngx_http_next_body_filter(r, in);
    }

    //从请求的内存池中分配内存，用于存储字符串前缀
    ngx_buf_t* b = ngx_create_temp_buf(r->pool, filter_prefix.len);
    //将ngx_buf_t中的指针正确地指向filter_prefix字符串
    b->start = b->pos = filter_prefix.data;
    b->last = b->pos + filter_prefix.len;

    //从请求的内存池中生成ngx_chain_t链表，将刚分配的ngx_buf_t设置到
    //其buf成员中，并将它添加到原先待发送的http包体前面
    ngx_chain_t *cl = ngx_alloc_chain_link(r->pool);
    cl->buf = b;
    cl->next = in;

    //调用下一个模块的http包体处理方法，注意这时传入的是新生成的cl链表
    return ngx_http_next_body_filter(r, cl);
}
