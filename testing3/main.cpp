#include <iostream>
#include <functional>

typedef int oli;
typedef int (*r_fun)(int, int);


int m_sum(int a, int b)
{
    return a + b;
}

r_fun ret_fun() { 
    return m_sum; 
}

r_fun ret_fun_2(int g)
{
    std::cout << g << std::endl;
    r_fun ret = [](int h, int j) -> int {
        return h + j;
    };

    std::cout << &ret << std::endl;
    return ret;
}

std::function<int(int, int)> ret_fun_3(int g)
{
    auto m_fun = [g](int h, int j) -> int {
        return h + j + g;};

    std::cout << typeid(m_fun).name() << std::endl;
    std::cout << &m_fun << std::endl;

    return m_fun;
}

//void print_callback(int cb(int, int))
//{
//    std::cout << cb(8, 8) << std::endl;
//}

template <typename T>
void print_callback(T cb)
{
    std::cout << "__std::function__\n";
    std::cout << cb(8, 8) << std::endl;
}

int main()
{
    std::cout << ret_fun() << std::endl;
    std::cout << m_sum << std::endl;
    std::cout << ret_fun_2(5) << std::endl;

    auto f = ret_fun_3(5);
    auto g = ret_fun_3(10);

    std::cout << &f << std::endl;

    print_callback(g);
    print_callback(m_sum);

    std::cout << f(5, 5) << std::endl;
    std::cout << g(5, 5) << std::endl;

    return 0;
}