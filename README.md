practice-nginx-module
====

练习 Nginx 模块开发知识点

安装
===
- clone项目：https://github.com/WuHuaJi0/practice-nginx-module
- 重 configure Nginx，并指定 `practice-nginx-module`目录：`./configure --prefix=./compile  --add-module=./practice-nginx-module`
- 编译和安装：make && make install

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
