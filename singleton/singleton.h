#pragma once
#include <memory.h>

template <class T>
class Singleton {
public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static std::shared_ptr<T> instance()
    {
        static std::shared_ptr<T> s{new T};
        return s;
    }

private:
    Singleton() {}
};