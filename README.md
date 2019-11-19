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
* [todo](#todo)


print
======
可用`print`指令输出内容，格式：`print 参数1 参数2 ***`,可带任意个参数，将响应参数内容:
 
```
location /print{
    print hello world nihao shijie;
}
```

响应：
```
curl http://localhost/print
$ hello world nihao shijie;
```


todo
======
- [ ] print 可输出变量，如`print $request_uri`;
