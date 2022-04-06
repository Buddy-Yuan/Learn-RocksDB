## 前言

最近看了一篇文章挺有意思的，叫**《RocksDB Is Eating the Database World》**。如下图，`RocksDB` 正在吞噬着数据库世界。

![](https://pic.imgdb.cn/item/62276e435baa1a80ab8c6374.png)

Why ？

我个人觉得有以下几点，

- 重新设计一个存储引擎是一个极其复杂的事情，而且会浪费巨大的人力和物力，同时开发周期也会变得漫长。但是自己设计存储引擎也有极大的好处，技术可控，并且可以和上层绑定进行优化，发挥出最大的性能。

- `RocksDB`出身名门，且在负载较高的`FaceBook`环境下稳定运行，足以说明它是可以应付大规模应用，可靠性强。
- 性能出色。`RocksDB`在密集型的基准测试中表现非常亮眼，[MyRocks](http://myrocks.io/)项目是 MySQL 的一个分支，它用 `RocksDB` 作为 MySQL 的存储引擎来代替 InnoDB，Mark Callaghan 进行了广泛而严格的测试。

![](https://pic.imgdb.cn/item/62276e715baa1a80ab8c7bd6.png)

![](https://pic.imgdb.cn/item/62276e835baa1a80ab8c8496.png)

可以看到在写入密集型基准测试中，因为有`LSM Tree`的能力加持，可以完胜innodb。但在读上面，因为写放大的问题，使得它在读取密集型基准测试要落后于innodb。但是两者在读的差距并没有写那么大。

从WiKi上看，当前`Rocksdb`主要使用是在`替代后端`和`嵌入式应用`。

![](https://pic.imgdb.cn/item/62276e905baa1a80ab8c8961.png)

## RocksDB 安装

初步了解了`RocksDB`之后，我们还是先把它装起来。安装有一个捷径可以选择，参考github的一键安装脚本。

https://gist.github.com/srimaln91/bea462f8e3cefc793125757a3080391f

你也可以手动一步一步的执行。

> ⚠️ 注意
>
> 当前RocksDB的版本是6.29.3，但是在CentOS 7版本上编译会遇到一个issue。
>
> https://github.com/tecbot/gorocksdb/issues/203
>
> 建议安装6.15.x或者早期版本，我选择是6.15.5

```shell
ROCKSDB_VERSION="6.15.5"
ZSTD_VERSION="1.5.2"

yum install -y \
  wget \
  gcc-c++ \
  snappy snappy-devel \
  zlib zlib-devel \
  bzip2 bzip2-devel \
  lz4-devel \
  libasan \
  gflags

wget -qO /tmp/zstd-${ZSTD_VERSION}.tar.gz https://github.com/facebook/zstd/archive/v${ZSTD_VERSION}.tar.gz
wget -qO /tmp/rocksdb-${ROCKSDB_VERSION}.tar.gz https://github.com/facebook/rocksdb/archive/v${ROCKSDB_VERSION}.tar.gz

cd /tmp

tar xzvf zstd-${ZSTD_VERSION}.tar.gz
tar xzvf rocksdb-${ROCKSDB_VERSION}.tar.gz

echo "Installing ZSTD..."
pushd zstd-${ZSTD_VERSION}
make && make install
popd

echo "Compiling RocksDB..."
pushd rocksdb-${ROCKSDB_VERSION}
make shared_lib
make install-shared INSTALL_PATH=/usr/local

# cleanup
rm -rf /tmp/zstd-${ZSTD_VERSION}.tar.gz /tmp/zstd-${ZSTD_VERSION}
rm -rf /tmp/rocksdb-${ROCKSDB_VERSION}.tar.gz /tmp/rocksdb-${ROCKSDB_VERSION}

echo "Installation Completed!"
```

把版本号和路径稍微调整一下，选择自己要安装的版本和位置，然后后台一键安装。

## 用Golang进行测试

安装完成之后，我们可以使用golang大法来进行测试。因为是嵌入式数据库，所以这里直接在go程序中进行使用。

> ⚠️ 注意
>
> 需要设置下面的环境变量，并安装gorocksdb

```go
CGO_CFLAGS="-I/usr/local/include/rocksdb/include" \
CGO_LDFLAGS="-L/usr/local/include/rocksdb -lrocksdb -lstdc++ -lm -lz -lbz2 -lsnappy -llz4 -lzstd" \
go get github.com/tecbot/gorocksdb
```

安装完成后，我们写一个go语言程序。

```go
package main

import (
"github.com/tecbot/gorocksdb"
"log"
)

func main(){
  options := gorocksdb.NewDefaultOptions()
  options.SetCreateIfMissing(true)
        db, err := gorocksdb.OpenDb(options, "/tmp/gorocksdb1")
        if err != nil {
        log.Println("fail to open db,", nil, db)
        }else
        {
                log.Println("open db success", nil, db)
        }
  db.Close()
}
```

这个程序极其的简单，就是打开数据库。

```go
go run c1.go
/tmp/go-build579019632/b001/exe/c1: error while loading shared libraries: librocksdb.so.6.15: cannot open shared object file: No such file or directory
exit status 127
```

运行报错，这里需设置环境变量`LD_LIBRARY_PATH`指向它需要的`librocksdb.so.6.15`的路径。

```go
export LD_LIBRARY_PATH=/usr/local/lib/
go run c1.go
2022/03/08 22:48:53 open db success <nil> &{0x2740420 /tmp/gorocksdb1 0xc00005a040}
```

显示数据库打开成功，我们去看一下。

```shell
[root@copy-of-vm-ee-centos76-v1 gorocksdb1]# ls -lrt
total 148
-rw-r--r-- 1 root root     0 Mar  8 22:48 LOCK
-rw-r--r-- 1 root root    37 Mar  8 22:48 IDENTITY
-rw-r--r-- 1 root root    16 Mar  8 22:48 CURRENT
-rw-r--r-- 1 root root     0 Mar  8 22:48 000004.log
-rw-r--r-- 1 root root  6062 Mar  8 22:48 OPTIONS-000006
-rw-r--r-- 1 root root    57 Mar  8 22:48 MANIFEST-000003
-rw-r--r-- 1 root root 23272 Mar  8 22:48 LOG
```

原来，嵌入式数据库就是这么玩的。

## 后记

今天只是对RocksDB的一个小小的认识和初步的尝试，后续将带来更多精彩内容。

## Refenerce

https://gist.github.com/srimaln91/bea462f8e3cefc793125757a3080391f

https://en.wikipedia.org/wiki/RocksDB#Alternative_backend

https://rockset.com/blog/rocksdb-is-eating-the-database-world/
