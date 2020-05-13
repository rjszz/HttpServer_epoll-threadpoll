# HttpServer_epoll-threadpoll
采用epoll+线程池的架构，实现的简单的httpserver



环境要求：C++ 11,支持多线程

`source`文件夹存放着html资源文件，需要一并下载



使用：

 为了匹配环境，请先使用`make`生成新的`httpserver`

 启动时可以输入参数`post`表示使用的通信端口，示例：

```
./httpserver	#默认使用 ip 0.0.0.0 port 8000 作为通信端口
./httpserver --ip 127.0.0.1 --port 8001	#使用 127.0.0.0.1 8001 作为通信地址
```

功能：

- 1、处理用户端的GET和POST请求

- 2、GET指令形如：

  ```
  curl -i -X GET http://127.0.0.1:8000/index.html
  ```

  如果请求成功会返回 index.html

  否则返回404页面

- 3、POST指令形如：

  ```
  curl -i -X POST --data 'Name=HNU&ID=CS06142' http://127.0.0.1:8000/Post_show
  ```

  需要注意的是，参数`Name`与`ID`以及后面的请求路径`Post_show`必须填写正确，否则会返回404

- 4、如果使用其他方法，会返回501



架构：

使用epoll监听多个文件描述符，当有客户端请求时，epoll检测到客户端的接入，并且将文件描述符添加到工作队列

线程池中的线程会去探勘工作队列中的是否有在等待的任务，如果有，就取出，并开始运行，如果没有就睡眠，等待唤醒。