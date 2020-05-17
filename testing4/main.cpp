#include <iostream>
#include <vector>
#include <string>
#include<experimental/generator>

std::experimental::generator<int> get_integers(int start = 0, int step = 1) {
    for (int current = start; true; current += step)
        co_yield current;
}


template<typename T>
class Ring
{
private:
    int m_pos;
    int m_size;
    T *m_values;

public:
    class iterator;

public:
    Ring(int size) : m_size(size), m_pos(0), m_values(nullptr)
    {
        m_values = new T[size];
    }

    ~Ring()
    {
        delete[] m_values;
    }

    int size() {
        return m_size;
    }
    
    iterator begin()
    {
        return iterator(0, *this);
    }

    iterator end()
    {
        return iterator(m_size, *this);
    }

    void add(T value)
    {
        m_values[m_pos++] = value;

        if (m_pos == m_size) m_pos = 0;
    }

    T &get(int pos)
    {
        return m_values[pos];
    }
};

template<class T>
class Ring<T>::iterator
{
private:
    Ring& m_ring;
    int m_pos;

public:
    iterator(int pos, Ring& aRing) : m_pos(pos), m_ring(aRing)
    {}

    iterator& operator++(int)
    {
        m_pos++;
        return *this;
    }

    iterator& operator++()
    {
        m_pos++;
        return *this;
    }

    T& operator*()
    {return m_ring.get(m_pos);}

    bool operator!=(const iterator& other)
    {
        return m_pos != other.m_pos;
    }
};

int main()
{

    auto counter = get_integers();

    auto j = counter.begin();
    std::cout << *j << std::endl;

    //std::cout << j << std::endl;
    std::cout << typeid(j).name() << std::endl;
    std::cout << typeid(*j).name() << std::endl;

    std::cout << "-----\n";

    j++;
    std::cout << *j << std::endl;

    std::cout << "-----\n";

    j++;
    std::cout << *j << std::endl;

    std::cout << "-----\n";

    for (auto i : counter)
    {
        std::cout << i << std::endl;
        break;
    }

    for (auto i : counter)
    {
        std::cout << i << std::endl;
        break;
    }

    
    //curtom iterator
    Ring<std::string> ring(3);
    ring.add("one");
    ring.add("two");
    ring.add("three");

    for (auto & it : ring)
    {
        std::cout << it << std::endl;
    }

    system("pause");

    return 0;

}