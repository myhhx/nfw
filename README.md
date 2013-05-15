介绍
----

nfw是用c++11和boost::asio编写的网络转发工具。

nfw非常适合用来学习c++11和boost::asio，因为它很小，功能又简单明了。

nfw非常小，总共12个文件，共1176行（其中和安全有关的代码为500行左右），每个函数（不包括main函数）不超过20行。

用法
----

    [myhhx@lovelywife bin]$ ./nfw_server_2262_linux_x86_64
    exception: nfw_assert(argc >= 3) [main:nfw_server.cpp:20]
    example: nfw_server 0.0.0.0 12345 --user name key --entry http a.com 80 --entry https a.com 443 --entry ssh a.com 22
    [myhhx@lovelywife bin]$ ./nfw_client_2261_linux_x86_64
    exception: nfw_assert(argc >= 5) [main:nfw_client.cpp:19]
    example: nfw_client a.com 12345 user-name user-key --entry 0.0.0.0 80 http --entry 0.0.0.0 443 https --entry 0.0.0.0 22 ssh

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

联系
---

ｑｑ：　　1285426900

ｍａｉｌ：myhhx@hotmail.com

ｗｗｗ：　http://www.myhhx.com
