// 小彭老师作业05：假装是多线程 HTTP 服务器 - 富连网大厂面试官觉得很赞
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <string>


struct User {
    std::string password;
    std::string school;
    std::string phone;
};

class HTTPServer {
private:
    std::map<std::string, User> users;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock> > has_login;  // 换成 std::chrono::seconds 之类的
    std::mutex mtx;

public:
// 作业要求1：把这些函数变成多线程安全的
// 提示：能正确利用 shared_mutex 加分，用 lock_guard 系列加分
    std::string do_register(std::string username, std::string password, std::string school, std::string phone) {
        std::unique_lock lock(mtx);
        User user = {password, school, phone};
        if (users.emplace(username, user).second)
            return "注册成功";
        else
            return "用户名已被注册";
    }

    std::string do_login(std::string username, std::string password) {
        // 作业要求2：把这个登录计时器改成基于 chrono 的
        std::unique_lock lock(mtx);
        auto now = std::chrono::steady_clock::now();
        if (has_login.find(username) != has_login.end()) {
            auto sec = now - has_login.at(username);  // C 语言算时间差
            return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(sec).count()) + "秒内登录过";
        }
        has_login[username] = now;

        if (users.find(username) == users.end())
            return "用户名错误";
        if (users.at(username).password != password)
            return "密码错误";
        return "登录成功";
    }

    std::string do_queryuser(std::string username) {
        std::unique_lock  lock(mtx);
        if (users.find(username) == users.end()) {
            return "unknown";
        }
        auto &user = users.at(username);
        std::stringstream ss;
        ss << "用户名: " << username << std::endl;
        ss << "学校:" << user.school << std::endl;
        ss << "电话: " << user.phone << std::endl;
        return ss.str();
    }
};

HTTPServer http_server;


struct ThreadPool {
private:
    std::vector<std::thread> m_pool;
    std::mutex mtx;
public:
    void create(std::function<void()> start) {
        // 作业要求3：如何让这个线程保持在后台执行不要退出？
        // 提示：改成 async 和 future 且用法正确也可以加分
        std::unique_lock<std::mutex> lock;
        if (m_pool.size() >= 4) {
            for (auto& thr: m_pool) {
                thr.join();
            }
            m_pool.clear();
        }
        m_pool.push_back(std::move(std::thread(start)));
    }

    ~ThreadPool() {
        for (auto& thr : m_pool) {
            thr.join();
        }
    }
};

ThreadPool tpool;


namespace test {  // 测试用例？出水用力！
std::string username[] = {"张心欣", "王鑫磊", "彭于斌", "胡原名"};
std::string password[] = {"hellojob", "anti-job42", "cihou233", "reCihou_!"};
std::string school[] = {"九百八十五大鞋", "浙江大鞋", "剑桥大鞋", "麻绳理工鞋院"};
std::string phone[] = {"110", "119", "120", "12315"};
}

int main() {
    for (int i = 0; i < 262144; i++) {
        tpool.create([&] {
            std::cout << http_server.do_register(test::username[rand() % 4], test::password[rand() % 4], test::school[rand() % 4], test::phone[rand() % 4]) << std::endl;
        });
        tpool.create([&] {
            std::cout << http_server.do_login(test::username[rand() % 4], test::password[rand() % 4]) << std::endl;
        });
        tpool.create([&] {
            std::cout << http_server.do_queryuser(test::username[rand() % 4]) << std::endl;
        });
    }

    // 作业要求4：等待 tpool 中所有线程都结束后再退出 # done
    return 0;
}
