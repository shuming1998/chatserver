# C++ 集群聊天服务器

**项目介绍**

1. 技术选型

  (1) 网络模块： muduo 网络库

  (2) 消息序列化/反序列化：JSON

  (3) 负载均衡：nginx 负载均衡器

  (4) 服务器集群通信：Redis 消息队列

  (5) 数据持久化存储：MySQL

2. 功能支持

  (1) 登录/注册

  (2) 添加好友/群组

  (3) 好友/群组聊天

  (4) 离线消息

**数据库表设计**

**User 表**

| 字段名称 | 字段类型                  | 字段说明     | 约束                        |
| :------: | ------------------------- | ------------ | --------------------------- |
|    id    | INT                       | 用户id       | PRIMARY KEY、AUTO_INCREMENT |
|   name   | VARCHAR(50)               | 用户名       | NOT NULL、UNIQUE            |
| password | VARCHAR(50)               | 用户密码     | NOT NULL                    |
|  state   | ENUM('online', 'offline') | 当前登录状态 | DEFAULT 'offline'           |

**Friend 表**

| 字段名称 | 字段类型 | 字段说明 | 约束               |
| :------: | -------- | -------- | ------------------ |
|  userid  | INT      | 用户id   | NOT NULL、联合主键 |
| friendid | INT      | 好友id   | NOT NULL、联合主键 |

**AllGroup 表**

| 字段名称  | 字段类型     | 字段说明   | 约束                        |
| :-------: | ------------ | ---------- | --------------------------- |
|    id     | INT          | 组id       | PRIMARY KEY、AUTO_INCREMENT |
| groupname | VARCHAR(50)  | 组名称     | NOT NULL、UNIQUE            |
| groupdesc | VARCHAR(200) | 组功能描述 | DEFAULT ''                  |

**GroupUser 表**

| 字段名称  | 字段类型                  | 字段说明 | 约束               |
| :-------: | ------------------------- | -------- | ------------------ |
|  groupid  | INT                       | 组id     | NOT NULL、联合主键 |
|  userid   | INT                       | 组员id   | NOT NULL、联合主键 |
| grouprole | ENUM('creator', 'normal') | 组内角色 | DEFAULT 'normal'   |

**OfflineMessage 表**

| 字段名称 | 字段类型    | 字段说明                   | 约束        |
| :------: | ----------- | -------------------------- | ----------- |
|  userid  | INT         | 用户id                     | PRIMARY KEY |
| message  | VARCHAR(50) | 离线消息(存储 Json 字符串) | NOT NULL    |



**项目依赖**

1. JSON (https://github.com/nlohmann/json)

   ```shell
   $ git clone git@github.com:nlohmann/json.git
   $ sudo cp -r include/nlohmann usr/include
   ```

2. muduo (https://github.com/chenshuo/muduo)

   ```shell
   $ sudo apt install g++ cmake make libboost-dev
   $ git clone -b cpp11 git@github.com:chenshuo/muduo.git
   $ cd muduo
   $ vim CMakeLists.txt
     # option(MUDUO_BUILD_EXAMPLES "Build Muduo examples" ON) 注释该行，不编译样例
   $ ./build.sh
   $ ./build.sh install    # muduo 编译后头文件和库文件都不在系统路径下
   $ cd build/release-install-cpp11/include
   # 将 muduo 头文件拷贝到 chatserver/include/server 目录中
   $ cp -r muduo [your path]/chatserver/include/sever/
   $ cd ../lib
   # 将本项目用到的 muduo 静态库文件拷贝到 chatserver/thirdparty 目录中
   $ cp libmuduo_base.a [your path]/chatserver/thirdparty/
   $ cp libmuduo_net.a [your path]/chatserver/thirdparty/
   ```

3. nginx (https://github.com/nginx/nginx)

   ```shell
   # nginx 1.12.2
   $ sudo apt-get install pcre openssl zlib
   $ git clone git@github.com:nginx/nginx.git
   $ cd nginx
   $ ./configure --with-stream
   $ make -j32
   $ sudo make install
   # 配置 nginx 负载均衡
   $ cd conf
   $ vim nginx.conf
   # 配置完成后平滑重启 nginx
   nginx -s reload
   ```

4. Redis (https://github.com/redis/hiredis)

   ```shell
   $ sudo apt-get install redis-server
   # redis-client C++ API (0.14.0)
   $ git clone git@github.com:redis/hiredis.git
   $ cd hiredis
   $ make -j32
   $ sudo make install
   $ sudo mkdir -p /usr/local/include/hiredis/adapters
   $ sudo cp -r hiredis.h async.h read.h sds.h sslio.h /usr/local/include/hiredis
   $ sudo cp libhiredis.so libhiredis.a /usr/local/lib/
   $ sudo mkdir -p /usr/local/lib/pkgconfig
   $ sudo cp hiredis.pc /usr/local/lib/pkgconfig
   $ sudo ldconfig
   ```

5. MySQL (https://github.com/mysql/mysql-server)

   ```shell
   $ sudo apt-get install mysql-server libmysqlclient-dev
   $ mysql -u root -p
   # 查看默认字符编码
   $ show variables like "char%";
   # 设置字符编码支持中文
   $ set character_set_server=utf8;
   ```

   

**编译**

```shell
$ git clone git@github.com:shuming1998/chatserver.git
$ cd chatserver
$ ./autobuild.sh
```



**测试**

```shell
# server
$ cd bin
$ ./ChatServer 127.0.0.1 6666
```



```shell
# client
$ cd chatserver/bin
$ ./ChatClient 127.0.0.1 6666
```



