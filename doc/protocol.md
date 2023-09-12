协议文档：
以下为 XProxys 和 XProxyc 通信的相关协议：
整个通信协议采用以下格式：

| 命令   | 长度   | 检验和   | 数据   | 尾     |
| ------ | ------ | -------- | ------ | ------ |
| cmd    | len    | checksum | param  | tail   |
| 2 byte | 2 byte | 2 byte   | N byte | 1 byte |

数据帧的长度是除了尾的之外的各个字段的数据长度之和。

其中数据帧的尾为 `0x7E` ，若整个数据帧中存在 `0x7E` 时，需要进行数据 padding 以避免出现 `0x7E` ，其具体的方法如下：

加入填充数据 `0x7D` 然后将原始数据和异或 `0x20` ，然后将这两个字节的数据替换原始数据，示例如下：

若存在数据：

```
0x01 0x02 0x7E
```

经过转换后，变为：

```
0x01 0x02 0x7D 0x5E
```

需要注意的是：数据帧中的 `0x7D` 也需要进行 padding .



#### 详细命令

##### XProxyc 登陆命令

该命令是由 `XProxyc` 向 `XProxys` 发送

命令格式如下：

| cmd    | len    | checksum | param  | tail   |
| ------ | ------ | -------- | ------ | ------ |
| 0x0000 | X      | X        | X      | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | N Byte | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* param：该命令的参数，其格式为传递给 `XProxyc` 的配置文件决定，其内容是一个合法的 json 字符串
* tail ：数据帧的尾

该命令的返回如下：

| cmd    | len    | checksum | result | tail  |
| ------ | ------ | -------- | ------ | ----- |
| 0x8000 | 7      | X        | x      | 0x7e  |
| 2 Byte | 2 Byte | 2 Byte   | 1 Byte | 1 Bye |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* param：
  * result ：该结果为登陆情况：
    * 0 ：登陆成功
    * 1：登陆失败
* tail ：数据帧的尾



##### XProxyc 登出命令

该命令是由 `XProxyc` 向 `XProxys` 发送

命令格式如下：

| cmd    | len    | checksum | param  | tail   |
| ------ | ------ | -------- | ------ | ------ |
| 0x0001 | X      | X        | null   | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 0 Byte | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* param：无
* tail ：数据帧的尾

该命令的返回如下：

| cmd    | len    | checksum | param  | tail  |
| ------ | ------ | -------- | ------ | ----- |
| 0x8001 | X      | X        | null   | 0x7e  |
| 2 Byte | 2 Byte | 2 Byte   | 0 Byte | 1 Bye |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* param：无
* tail ：数据帧的尾



##### XProxys 对外的服务有客户端连接上

该命令是由 `XProxys` 向 `XProxyc` 发送

命令格式如下：

| cmd    | len    | checksum | client id | service id | tail   |
| ------ | ------ | -------- | --------- | ---------- | ------ |
| 0x0010 | X      | X        | X         | X          | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 4 Byte    | 4 Byte     | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* client id：连接上的客户端 ID  -- 用于区分不同的客户端
* service id ：要连接的服务 ID   -- 用于区分不同的服务
* tail ：数据帧的尾

注意：为了整体性能的考虑，接受到该命令的客户端不需要进行回复



##### XProxys 服务的客户端向服务发送数据

该命令是由 `XProxys` 向 `XProxyc` 发送

命令格式如下：

| cmd    | len    | checksum | client id | service id | data   | tail   |
| ------ | ------ | -------- | --------- | ---------- | ------ | ------ |
| 0x0011 | X      | X        | X         | X          | X      | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 4 Byte    | 4 Byte     | N Btye | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* client id：连接上的客户端 ID  -- 用于区分不同的客户端
* service id ：要连接的服务 ID   -- 用于区分不同的服务
* data : 实际需要发送的数据
* tail ：数据帧的尾

注意：为了整体性能的考虑，接受到该命令的客户端不需要进行回复



##### XProxyc 服务有数据返回

该命令是由 `XProxyc` 向 `XProxys` 发送

命令格式如下：

| cmd    | len    | checksum | client id | service id | data   | tail   |
| ------ | ------ | -------- | --------- | ---------- | ------ | ------ |
| 0x8011 | X      | X        | X         | X          | X      | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 4 Byte    | 4 Byte     | N Btye | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* client id：连接上的客户端 ID  -- 用于区分不同的客户端
* service id ：要连接的服务 ID   -- 用于区分不同的服务
* data : 实际需要发送的数据
* tail ：数据帧的尾

注意：为了整体性能的考虑，接受到该命令的服务端不需要进行回复





##### XProxys 服务的客户端断开连接

该命令是由 `XProxys` 向 `XProxyc` 发送

命令格式如下：

| cmd    | len    | checksum | client id | service id | tail   |
| ------ | ------ | -------- | --------- | ---------- | ------ |
| 0x0012 | X      | X        | X         | X          | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 4 Byte    | 4 Byte     | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* client id：连接上的客户端 ID  -- 用于区分不同的客户端
* service id ：要连接的服务 ID   -- 用于区分不同的服务
* data : 实际需要发送的数据
* tail ：数据帧的尾

注意：为了整体性能的考虑，接受到该命令的客户端不需要进行回复





##### XProxyc 服务的客户端断开连接

该命令是由 `XProxyc` 向 `XProxys` 发送

命令格式如下：

| cmd    | len    | checksum | client id | service id | tail   |
| ------ | ------ | -------- | --------- | ---------- | ------ |
| 0x8012 | X      | X        | X         | X          | 0x7E   |
| 2 Byte | 2 Byte | 2 Byte   | 4 Byte    | 4 Byte     | 1 Byte |

* cmd ：该命令的ID
* len：该命令的长度，会根据实际的 param 来进行填充
* checksum ：该命令的校验和，会根据实际的 param 来进行填充
* client id：连接上的客户端 ID  -- 用于区分不同的客户端
* service id ：要连接的服务 ID   -- 用于区分不同的服务
* data : 实际需要发送的数据
* tail ：数据帧的尾

注意：为了整体性能的考虑，接受到该命令的客户端不需要进行回复



