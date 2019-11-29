#include "ngx_http_practice_module.h"
#include "ngx_http_practice_upstream.h"
#include "ngx_http_practice_subrequest.h"

// set 函数；
char *ngx_http_sub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_sub_handler;
    return NGX_CONF_OK;
}

static ngx_int_t subrequst_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc) {
    ngx_http_request_t *pr = r->parent; //找到父请求；
    /*设置上下文*/
    ngx_http_sub_ctx_t * myctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);

    pr->headers_out.status = r->headers_out.status;

    if (r->headers_out.status == NGX_HTTP_OK) {
        ngx_buf_t *pRecvBuf = &r->upstream->buffer;
        myctx->res.data = pRecvBuf->pos;
        myctx->res.len = pRecvBuf->last - pRecvBuf->pos;
        pr->write_event_handler = sub_post_handler ; //设置父请求的回调
    }
    return NGX_OK;
}

//父请求的回调
static void sub_post_handler(ngx_http_request_t *r){
    if( r->headers_out.status != NGX_HTTP_OK ){
        ngx_http_finalize_request(r,r->headers_out.status);
        return ;
    }

    /*设置上下文*/
    ngx_http_sub_ctx_t * myctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);

    ngx_str_t output = myctx->res;

    r->headers_out.content_length_n = myctx->res.len;
    ngx_buf_t *b = ngx_create_temp_buf(r->pool,myctx->res.len);

    b->pos = myctx->res.data;
    b->last = b->pos + myctx->res.len;
    b->last_buf = 1;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    static ngx_str_t type = ngx_string("text/plain");
    r->headers_out.content_type = type;
    r->headers_out.status = NGX_HTTP_OK;
    r->connection->buffered |= NGX_HTTP_WRITE_BUFFERED;

    ngx_int_t ret = ngx_http_send_header(r);
    ret = ngx_http_output_filter(r,&out);

    ngx_http_finalize_request(r,ret);

}

// handler函数
ngx_int_t ngx_http_sub_handler(ngx_http_request_t *r) {

    /*设置上下文*/
    ngx_http_sub_ctx_t * myctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);
    if (myctx == NULL) {
        myctx = ngx_palloc(r->pool, sizeof(ngx_http_sub_ctx_t));
        if (myctx == NULL) {
            return NGX_ERROR;
        }
        ngx_http_set_ctx(r, myctx, ngx_http_practice_module);
    }

    ngx_http_post_subrequest_t *psr = ngx_palloc(r->pool, sizeof(ngx_http_post_subrequest_t));
    if( psr == NULL ){
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    psr->handler = subrequst_post_handler;
    psr->data = myctx;
    ngx_str_t sub_prefix = ngx_string("/print");

    ngx_http_request_t *sr;
    ngx_int_t rc = ngx_http_subrequest(r,&sub_prefix, NULL, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY);
    if( rc != NGX_OK ){
        return NGX_ERROR;
    }

    return NGX_DONE;
}

