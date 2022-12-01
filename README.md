# Clustered-live-chatServer-with-load-balancer

#### Environment:

```
$ sudo apt-get install gcc-7
$ sudo apt-get install g++-7
$ sudo apt-get install mysql-server
$ sudo apt-get install libmysqlclient-dev
$ sudo apt-get install redis-server
```

**hiredis version:0.14.0**

**nginx version:1.12.2**

#### How to run:

```shell
$ cd Clustered-live-chatServer-with-load-balancer
$ ./autobuild.sh
$ cd bin
$ ./ChatServer
```

another terminal:

```shell
$ cd Clustered-live-chatServer-with-load-balancer/bin
$ ./ChatClient
```

