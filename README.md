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
* [add_content(练习filter)](#add_content)
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


add_content
======
可用`add_content`指令在响应体前添加一段：`[content add by filter]`，主要练习 nginx http 过滤器。
指令：`add_content on|off`;

```
location /print{
    print hello world;
    add_content on;
}
```

响应：
```
$ curl http://localhost/print 
[content add by filter]
hello world  
```

todo
======
- [ ] print 可输出变量，如`print $request_uri`;
