//
// Created by 吴化吉 on 11/23/19.
//

#include "ngx_http_practice_module.h"
#include "ngx_http_practice_upstream.h"

// set 函数；
char *ngx_http_upstream(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_upstream_handler;
    return NGX_CONF_OK;
}

// handler函数
ngx_int_t ngx_http_upstream_handler(ngx_http_request_t *r) {

    /*设置上下文*/
    ngx_http_practice_ctx_t *myctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);
    if (myctx == NULL) {
        myctx = ngx_palloc(r->pool, sizeof(ngx_http_practice_ctx_t));
        if (myctx == NULL) {
            return NGX_ERROR;
        }
        ngx_http_set_ctx(r, myctx, ngx_http_practice_module);
    }

    //给请求添加upstream属性 ,具体查看 ngx_http_upstream.c，
    if (ngx_http_upstream_create(r) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_http_upstream_create fialed");
        return NGX_ERROR;
    }

    // 添加 upstream 配置
    ngx_http_practice_loc_conf_t *mycf = ngx_http_get_module_loc_conf(r, ngx_http_practice_module); //读取到模块配置
    ngx_http_upstream_t *u = r->upstream;
    u->conf = &mycf->upstream_conf; //设置upstrem配置
    u->buffering = mycf->upstream_conf.buffering;

    //设置上游服务器
    u->resolved = (ngx_http_upstream_resolved_t *) ngx_pcalloc(r->pool, sizeof(ngx_http_upstream_resolved_t));
    if (u->resolved == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_pcalloc resolved error");
        return NGX_ERROR;
    }

    static struct sockaddr_in backendSockAddr;
    struct hostent *pHost = gethostbyname((char *) "localhost");
    if (pHost == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gethostbyname failed");
        return NGX_ERROR;
    }

    backendSockAddr.sin_family = AF_INET;
    backendSockAddr.sin_port = htons((in_port_t) 80);

    char *pDmsIP = inet_ntoa(*(struct in_addr *) (pHost->h_addr_list[0]));
    backendSockAddr.sin_addr.s_addr = inet_addr(pDmsIP);

    myctx->backendServer.data = (u_char *) pDmsIP;
    myctx->backendServer.len = strlen(pDmsIP);

    u->resolved->sockaddr = (struct sockaddr *) &backendSockAddr;
    u->resolved->socklen = sizeof(struct sockaddr_in);
    u->resolved->port = 80;
    u->resolved->naddrs = 1;

    u->create_request = practice_upstream_create_request;
    u->process_header = practice_process_status_line;
    u->finalize_request = practice_upstream_finialize_request;

    r->main->count++; //确保请求不会被销毁；
    ngx_http_upstream_init(r);
    return NGX_DONE;
}

// upstream 的 create_request 方法；
ngx_int_t practice_upstream_create_request(ngx_http_request_t *r) {

    //模拟请求
    static ngx_str_t backendQueryLine = ngx_string("GET /print HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\naccept: */*\r\n\r\n");

    ngx_buf_t *b = ngx_create_temp_buf(r->pool, backendQueryLine.len);  //初始化一个buf , 这里的buf存放要发给上游的请求
    if (b == NULL){
        return NGX_ERROR;
    }

    b->last = b->pos + backendQueryLine.len; // 疑问: b->pos 指向 backendQuery.data ?

    ngx_snprintf(b->pos, backendQueryLine.len, (char *) backendQueryLine.data, &r->args); //带入参数，把 %V 替换为要查询的参数


    r->upstream->request_bufs = ngx_alloc_chain_link(r->pool); //初始化 request_bufs ，它是一个 ngx_chain_t ;
    if (r->upstream->request_bufs == NULL)
        return NGX_ERROR;

    //把前面初始化的 buf 作为 request_bufs 的成员
    r->upstream->request_bufs->buf = b;
    r->upstream->request_bufs->next = NULL;

    r->upstream->request_sent = 0;
    r->upstream->header_sent = 0;
    // header_hash不可以为0
    r->header_hash = 1;
    printf("create request\n");
    return NGX_OK;
}



//解析响应行
ngx_int_t practice_process_status_line(ngx_http_request_t *r) {

    size_t len;
    ngx_int_t rc;
    ngx_http_upstream_t *u;

    //上下文中才会保存多次解析http响应行的状态，首先取出请求的上下文
    ngx_http_practice_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    u = r->upstream;

    //http框架提供的ngx_http_parse_status_line方法可以解析http
    //响应行，它的输入就是收到的字符流和上下文中的ngx_http_status_t结构
    rc = ngx_http_parse_status_line(r, &u->buffer, &ctx->status);
    //返回NGX_AGAIN表示还没有解析出完整的http响应行，需要接收更多的
    //字符流再来解析
    if (rc == NGX_AGAIN) {
        return rc;
    }
    //返回NGX_ERROR则没有接收到合法的http响应行
    if (rc == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "upstream sent no valid HTTP/1.0 header");
        r->http_version = NGX_HTTP_VERSION_9;
        u->state->status = NGX_HTTP_OK;

        return NGX_OK;
    }

    //以下表示解析到完整的http响应行，这时会做一些简单的赋值操作，将解析出
    //的信息设置到r->upstream->headers_in结构体中，upstream解析完所
    //有的包头时，就会把headers_in中的成员设置到将要向下游发送的
    //r->headers_out结构体中，也就是说，现在我们向headers_in中设置的
    //信息，最终都会发往下游客户端。为什么不是直接设置r->headers_out而要
    //这样多此一举呢？这是因为upstream希望能够按照
    //ngx_http_upstream_conf_t配置结构体中的hide_headers等成员对
    //发往下游的响应头部做统一处理
    if (u->state) {
        u->state->status = ctx->status.code;
    }

    u->headers_in.status_n = ctx->status.code;

    len = ctx->status.end - ctx->status.start;
    u->headers_in.status_line.len = len;

    u->headers_in.status_line.data = ngx_pnalloc(r->pool, len);
    if (u->headers_in.status_line.data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(u->headers_in.status_line.data, ctx->status.start, len);

    //下一步将开始解析http头部，设置process_header回调方法为
    //mytest_upstream_process_header，
    //之后再收到的新字符流将由mytest_upstream_process_header解析
    u->process_header = practice_upstream_process_header;

    //如果本次收到的字符流除了http响应行外，还有多余的字符，
    //将由mytest_upstream_process_header方法解析
    return practice_upstream_process_header(r);
}

ngx_int_t practice_upstream_process_header(ngx_http_request_t *r) {

    ngx_int_t rc;
    ngx_table_elt_t *h;
    ngx_http_upstream_header_t *hh;
    ngx_http_upstream_main_conf_t *umcf;

    //这里将upstream模块配置项ngx_http_upstream_main_conf_t取了
    //出来，目的只有1个，对将要转发给下游客户端的http响应头部作统一
    //处理。该结构体中存储了需要做统一处理的http头部名称和回调方法
    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);

    //循环的解析所有的http头部
    for (;;) {
        // http框架提供了基础性的ngx_http_parse_header_line
//方法，它用于解析http头部
        rc = ngx_http_parse_header_line(r, &r->upstream->buffer, 1);
        //返回NGX_OK表示解析出一行http头部
        if (rc == NGX_OK) {
            //向headers_in.headers这个ngx_list_t链表中添加http头部
            h = ngx_list_push(&r->upstream->headers_in.headers);
            if (h == NULL) {
                return NGX_ERROR;
            }
            //以下开始构造刚刚添加到headers链表中的http头部
            h->hash = r->header_hash;

            h->key.len = r->header_name_end - r->header_name_start;
            h->value.len = r->header_end - r->header_start;
            //必须由内存池中分配存放http头部的内存
            h->key.data = ngx_pnalloc(r->pool,
                                      h->key.len + 1 + h->value.len + 1 + h->key.len);
            if (h->key.data == NULL) {
                return NGX_ERROR;
            }

            h->value.data = h->key.data + h->key.len + 1;
            h->lowcase_key = h->key.data + h->key.len + 1 + h->value.len + 1;

            ngx_memcpy(h->key.data, r->header_name_start, h->key.len);
            h->key.data[h->key.len] = '\0';
            ngx_memcpy(h->value.data, r->header_start, h->value.len);
            h->value.data[h->value.len] = '\0';

            if (h->key.len == r->lowcase_index) {
                ngx_memcpy(h->lowcase_key, r->lowcase_header, h->key.len);
            } else {
                ngx_strlow(h->lowcase_key, h->key.data, h->key.len);
            }

            //upstream模块会对一些http头部做特殊处理
            hh = ngx_hash_find(&umcf->headers_in_hash, h->hash,
                               h->lowcase_key, h->key.len);

            if (hh && hh->handler(r, h, hh->offset) != NGX_OK) {
                return NGX_ERROR;
            }

            continue;
        }

        //返回NGX_HTTP_PARSE_HEADER_DONE表示响应中所有的http头部都解析
//完毕，接下来再接收到的都将是http包体
        if (rc == NGX_HTTP_PARSE_HEADER_DONE) {
            //如果之前解析http头部时没有发现server和date头部，以下会
            //根据http协议添加这两个头部
            if (r->upstream->headers_in.server == NULL) {
                h = ngx_list_push(&r->upstream->headers_in.headers);
                if (h == NULL) {
                    return NGX_ERROR;
                }

                h->hash = ngx_hash(ngx_hash(ngx_hash(ngx_hash(
                        ngx_hash('s', 'e'), 'r'), 'v'), 'e'), 'r');

                ngx_str_set(&h->key, "Server");
                ngx_str_null(&h->value);
                h->lowcase_key = (u_char *) "server";
            }

            if (r->upstream->headers_in.date == NULL) {
                h = ngx_list_push(&r->upstream->headers_in.headers);
                if (h == NULL) {
                    return NGX_ERROR;
                }
                h->hash = ngx_hash(ngx_hash(ngx_hash('d', 'a'), 't'), 'e');
                ngx_str_set(&h->key, "Date");
                ngx_str_null(&h->value);
                h->lowcase_key = (u_char *) "date";
            }

            return NGX_OK;
        }

        //如果返回NGX_AGAIN则表示状态机还没有解析到完整的http头部，
        //要求upstream模块继续接收新的字符流再交由process_header
        //回调方法解析
        if (rc == NGX_AGAIN) {
            return NGX_AGAIN;
        }

        //其他返回值都是非法的
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "upstream sent invalid header");

        return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }
}

void practice_upstream_finialize_request(ngx_http_request_t *r, ngx_int_t rc) {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "upstream finalize request");
}
