#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;
using namespace std;

json js;

// json序列化代码1
void func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "张三";
    js["to"] = "li si";
    js["msg"] = "hello what are you doing now?";

    string sendBuf = js.dump();

    cout << sendBuf << endl;
}

// json序列化代码2
void func2()
{
    json js;

    // 添加数组
    js["id"] = {1, 2, 3, 4, 5};

    // 添加key-value
    js["name"] = "zhang san";

    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";

    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};

    cout << js << endl;
}

// json序列化代码3
void func3()
{
    json js;

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    js["list"] = vec;

    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});

    js["path"] = m;

    cout << js << endl;
}

// json数据反序列化
void func4()
{

    // 普通序列化
    js["name"] = "zhang san";

    // 容器序列化
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    js["list"] = vec;

    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});

    js["path"] = m;

    // 将json数据序列化为json字符串
    // string sendBuf = js.dump();
    // cout << js.dump(1) << endl;

    string sendBuf = "asdas";

    /*********

     * 将字符串发送到网络中

    *********/

    // 模拟从网络接收到json字符串，通过json::parse函数把json字符串反序列化为json对象
    json js2 = json::parse(sendBuf);
    if (js2.is_discarded())
    {
        cout << 1 << endl;
        return;
    }

    // 直接取key-value
    string name = js2["name"];
    cout << "name = " << name << endl;

    // 反序列化为vector容器
    vector<int> v = js2["list"];
    for (int val : v)
    {
        cout << val << " ";
    }

    cout << endl;

    // 反序列化为map容器
    map<int, string> m2 = js2["path"];

    for (auto p : m2)
    {
        cout << p.first << " " << p.second << endl;
    }

    return;
}

// 将json数据写进文件中
void func5()
{
    func4();
    // 文件名称
    string path = "test.json";

    ofstream os;
    os.open(path, ios::out);
    if (!os.is_open())
    {
        cout << "文件打开失败..." << endl;
        return;
    }

    os << js.dump(4) << endl;
    return;
}

int main()
{
    // func1();

    // func2();

    func3();

    // func4();

    // func5();

    return 0;
}
