介绍
----

nfw是用c++11和boost::asio编写的网络转发工具。

nfw非常适合用来学习c++11和boost::asio，因为它很小，功能又简单明了。

nfw非常小，总共12个文件，共1176行（其中和安全有关的代码为500行左右），每个函数（不包括main函数）不超过20行。

用法
----

购买源码和相关资料
------------------

包括：README文档、BUILD文档、讲解视频和源码。

价格：999元人民币。

　

通过购买您可能获得的好处：

快速掌握c++11和boost::asio，提升编程技能，直接跨入万元月薪行列。

在自己的项目中使用这些源码，加速开发。

改造nfw成为自己的工具。

　

我希望以此能养家糊口，所以希望买家不要传播、散布您所购买的资料，谢谢！

展示
----

以下代码示例涉及到c++11的右值引用、move和lambda。

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
