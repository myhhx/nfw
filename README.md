介绍
----

nfw是用c++11和boost::asio编写的网络转发工具。

nfw非常适合用来学习c++11和boost::asio，因为它很小，功能又简单明了。

nfw非常小，总共12个文件，共1176行（其中和安全有关的代码为500行左右），每行代码不超过80个字符，每个函数（不包括main函数）不超过20行。

用法
----

就从使用案例开始吧。

二个环境，家中和公司。

家中，一个路由器，有公网ip，ip为1.1.1.1，一台内网电脑。
在路由器上做了映射，公网端口12345映射到内网电脑的12345端口。
在内网电脑上运行着ssh服务，服务端口为22。

在家中的内网电脑上运行

    nfw_server 0.0.0.0 12345 --user tom qazxswedc --entry ssh 127.0.0.1 22
    # 0.0.0.0 12345 是监听地址和监听端口
    # --user tom qazxswedc 是用户名和用户密钥
    # --entry ssh 127.0.0.1 22 是转发

这个命令的意思是，在12345端口上，如果有个标记为tom的新连接进来，就用对应的密钥去验证它，验证通过后，如果它还有个ssh的标记，就转发到本机的22端口。

公司，一台内网电脑，可上网。

在公司的内网电脑上运行

    nfw_client 1.1.1.1 12345 tom qazxswedc --entry 0.0.0.0 22 ssh
    # 1.1.1.1 12345 是连接到家中内网电脑的地址和端口
    # tom qazxswedc 是用户名和用户密钥
    # --entry 0.0.0.0 22 ssh 是转发

这个命令的意思是，在公司内网电脑上开启一个新服务，服务端口为22，当该服务有一个新的连接进来，就标记它为ssh，然后通过用户信息和连接信息转发到家中的内网电脑。

接下来，在公司内网电脑上用ssh客户端连接本机的22端口，就如同连到家中内网电脑的ssh服务。

如果你想机器启动时后台运行程序，如果是linux，一般在启动文件比如rc.local加入命令行即可，别忘了命令行结尾要加上“&”，如果你的linux用的是systemd，那么做个简单的service文件即可。在windows上，也很简单，有个sc工具，能够以服务方式运行普通程序。

购买
----

A

包括：README文档、BUILD文档、源码。

价格：99元人民币。

B

包括：README文档、BUILD文档、讲解视频和源码。

价格：999元人民币。

　

通过购买您可能获得的好处：

快速掌握c++11和boost::asio，提升编程技能，直接跨入万元月薪行列。

在自己的项目中使用这些源码，加速开发。

改造nfw成为自己的工具。

　

我希望以此能养家糊口，所以希望买家不要传播、散布您所购买的资料，谢谢！

示例
----

以下代码涉及到c++11的右值引用、move和lambda。

    template <typename handler_t>
    void resolve(boost::asio::io_service & io_service,
            const std::string & host, const std::string & service,
            handler_t && handler) {
        boost::asio::ip::tcp::resolver * resolver =
                new boost::asio::ip::tcp::resolver(io_service);
        resolver->async_resolve(
                boost::asio::ip::tcp::resolver::query(host, service),
                [resolver, &io_service, handler] (
                        const boost::system::error_code & error,
                        boost::asio::ip::tcp::resolver::iterator it) {
                    if (!error) {
                        connect(io_service, it, resolver, std::move(handler));
                    } else {
                        handler(NULL);
                        delete resolver;
                    }
                });
    }

计划
----

[ｘ] 基本功能

[ｘ] 安全性

[　] 反向连接

[　] client程序和server程序合并为一个程序

联系
---

ｑｑ：　　1285426900

ｍａｉｌ：myhhx@hotmail.com

ｗｗｗ：　http://www.myhhx.com
