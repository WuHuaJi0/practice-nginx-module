practice-nginx-module
====

练习 Nginx 模块开发知识点

目录
=================
* [print](#print)


print
======
`print 参数`可返回参数:
 
```
location /print{
    print hello;
}
```
响应：
```
curl http://localhost/print
$ hello
```
