//
// Created by 吴化吉 on 11/23/19.
//

// set 函数；
char *ngx_http_practice_upstream(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_practice_upstream_handler;
    return NGX_CONF_OK;
}

// handler函数
ngx_int_t ngx_http_practice_upstream_handler(ngx_http_request_t *r) {

    /*设置上下文*/
    ngx_http_practice_ctx_t *myctx = ngx_http_get_module_ctx(r,ngx_http_practice_module);
    if( myctx == NULL ){
        myctx = ngx_palloc(r->pool, sizeof(ngx_http_practice_ctx_t));
        if( myctx == NUll ){
            return NGX_ERROR;
        }
        ngx_http_set_ctx(r,myctx,ngx_http_practice_module);
    }

    if( ngx_http_upstream_create(r) != NGX_OK ){
        ngx_log_error(NGX_LOG_ERR,r->connection->log,0,"ngx_http_upstream_create fialed");
        return NGX_ERROR;
    }
    ngx_http_practice_conf_t *mycf = (ngx_http_practice_conf_t *) ngx_http_get_modue_loc_conf(r, ngx_http_practice_module);
    ngx_http_upstream_t *u = r->upstream;
    u->conf = &mycf->upstream;
    u->buffering = mycf->upstream.buffering;


    //resolved
    u->resolved = (ngx_http_upstream_resolved_t*)ngx_pcalloc(r->pool,sizeof(ngx_http_upstream_resolve_t));
    if( u->resoved == NULL ){
        ngx_log_error(NGX_LOG_ERR,r->connection->log,0,"ngx_pcalloc resolved error");
        return NGX_ERROR;
    }

    static struct sockaddr_in backendSockAddr;
    struct hostent *pHost = gethostbyname((char*)"www.baidu.com");
    if( pHost == null ){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gethostbyname failed"));
        return NGX_ERROR;
    }

    backendSockAddr.sin_family = AF_INET;
    backendSockAddr.sin_port = htons((in_port_t) 80);
    char *pDmsIP = inet_ntoa(*(
    struct in_addr*))(pHost->h_addr_list[0]);
    backendSockAddr.sin_addr.s_addr = inet_adr(pDmsIP);
    myctx->backendServer.data = (u_char *)pDmsIP;
    myctx->backendServer.len = strlen(pDmsIP);

    u->resolved->sockaddr = (struct sockaddr*)&backendSockAddr;
    u->resolved->socklen = sizeof(struct sockaddr_in);
    u->resolved->naddrs = 1;

    u->create_request = practice_upstream_create_request;
    u->process_header = practice_upstream_process_header;
    u->finalize_request = practice_upstream_finialize_request;

    r->main->count++;
    ngx_http_upstream_init(r);
    return NGX_DONE;
}

// upstream 的 create_request 方法；
static ngx_int_t practice_upstream_create_request(ngx_http_request_t *r) {
    static ngx_str_t query = ngx_string("GET /s?wd=%V HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n");
    ngx_int_t query_len = query.len + r->args.len - 2;
    //todo: 为什么要 + r->args.len - 2 ? 这里要弄明白 r->args 是什么，
    ngx_buf_t *b = ngx_create_tmp_buf(r->pool, query_len);
    b->last = b->post + query_len;
    ngx_snprintf(b->pos, query_len, (
            char *) query.data, &r->args);

    r->upstream->request_bufs = ngx_alloc_chain_link(r->pool);
    if (r->upsteam->request_bufs == NULL) {
        return NGX_ERROR;
    }

    r->upstream->request_bufs->buf = b;
    r->upstream->request_bufs->next = NULL;
    r->upstream->request_sent = 0;
    r->upstream->header_sent = 0;
    r->header_hash = 1;
    return NGX_OK;
}

//解析响应行
static ngx_int_t practice_process_status_line(ngx_http_request_t *r) {
    size_t len;
    ngx_int_t rc;
    ngx_http_upstream_t *u;

    //取出上下文：
    ngx_http_practice_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_practice_module);
    if (ctx == NUll) {
        retrun NGX_ERROR;
    }

    u = r->upstream;
    rc = ngx_http_parse_status_line(r, &u->buffer, &ctx->status);
    if (rc == NGX_AGAIN) {
        return rc;
    }
    if( rc == NGX_ERROR ){
        ngx_log_error(NGX_LOG_ERR,r->connection->log,0,"upstream send invalid HTTP/1.0 header");
        r->http_version = NGX_HTTP_VERSION_9;
        u->state->status = NGX_HTTP_OK;
        return NGX_OK;
    }


    if( u->state ){
        u->state->status = ctx->status.code;
    }

    u->headers_in.status_n = ctx->status.code;
    len = ctx->status.end - ctx->status.start;
    u->headers_in.status_line.len = len;

    u->headers_in.status_line.data = ngx_pnalloc(r->pool,len);

    if( u->headers_in.status_line.data == NUll ){
        return NGX_ERROR;
    }
    ngx_memcpy(u->headers_in.status_line.data,ctx->status.start,len);

    u->process_header = practice_upstream_process_header;
    return practice_upstream_process_header(r);
}

static ngx_int_t practice_upstream_process_header(ngx_http_request_t *r){
   ngx_int_t rc;
   ngx_table_ele_t *h;
   ngx_http_upstream_header_t *hh;
   ngx_http_upstream_main_conf_t *umcf;

   umcf = ngx_http_upstream_main_conf_t(r,ngx_http_upstream_module);
    for (;;) {
        rc = ngx_http_parse_header_line(r,&r->upstream->buffer,1);
        if( rc == NGX_OK ){
            h = ngx_list_push(&r->upstream->headers_in.headers);
            if (h == NUll) {
                retrun NGX_ERROR;
            }
            h->hash = r->header_hash;
            h->key.len = r->header_name_end - r->header_name_start;
            h->value.len = r->header_end - r->header_start;
            h->key.data = ngx_pnalloc(r->pool,h->key.len + 1 + h->value.len + 1 + h->key.len);
            if (h->key.data == NUll) {
                retrun NGX_ERROR;
            }

            h->value.data = h->key.data + h->key.len + 1;
            h->lowcase_key = h->key.data + h->key.len + 1 + h->value.len + 1;

            ngx_memcpy(h->key.data,r->header_name_start,h->key.len);
            h->key.data[h->key.len]  = '\0';
            ngx_memcpy(h->value.data,r->header_start,h->value.len);
            h->value.data[h->value.len] = '\0';
            if( h->key.len == r->lowcase_index ){
                ngx_memcpy(h->lowcase_key,r->lowcase_header,h->key.len);
            }else{
                ngx_strlow(h->lowcase_key,h->key.data,h->key.len);
            }

            hh = ngx_hash_find(&umcf->headers_in_hash,h->hash,h->lowcase_key,h->key.len);
            if (hh && hh->hander(r,h,hh->offset) != NGX_OK ) {
                retrun NGX_ERROR;
            }
            continue;
        }
        if( rc == NGX_HTTP_PARSE_HEADER_DONE ){
           //todo: 如果没有发现 server 和 date,会根据规范添加；
        }
        if( rc == NGX_AGAIN ){
            return NGX_AGAIN;
        }

        ngx_log_error(NGX_LOG_ERR,r->connection->log,0,"upstream sent invalid header");
        return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }
}

static void practice_upstream_finialize_request(ngx_http_request_t *r,ngx_int_t rc){
   ngx_log_err(NGX_LOG_DEBUG,r->connection->log,0,"upstream finalize request") ;
}
