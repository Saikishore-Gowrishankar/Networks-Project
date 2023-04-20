#pragma once
#include <memory>
#include <iterator>
#include <mutex>

template<typename T, unsigned SIZE>
struct chat_buffer
{
public:
    explicit chat_buffer() { buf_.reserve(SIZE); }
    
    template<typename U>
    void add(U&& item) 
    { 
        buf_.push_back(std::forward<U>(item));     
        if(size() > SIZE) buf_.erase(std::cbegin(buf_)); 
    }

    T& operator[] (size_t idx) 		   { return buf_[idx % SIZE];}
    T const& operator[] (size_t idx) const { return buf_[idx % SIZE]; }

    size_t size() const { return buf_.size(); }
    bool full() const { return buf_.size() == SIZE; }  
private:
    std::vector<T> buf_;
};

using ChatBuffer = chat_buffer<std::string, 33>;

