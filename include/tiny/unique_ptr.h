#include <memory>

template <typename T, typename D = std::default_delete<T>>
class unique_ptr {
    
private:
    T* internal_ptr = nullptr;
    D internal_deleter; 

public:
    unique_ptr() : internal_ptr(), internal_deleter() {}

    explicit unique_ptr(T* ptr) : internal_ptr(ptr) { // explicit is used on 1 arg to prevent type conversion prevent ptr accidental conversion to unique_ptr
    }

    unique_ptr(T* ptr, const D& deleter) : internal_ptr(ptr), internal_deleter(deleter) {}

    unique_ptr(T* ptr, D&& deleter) : internal_ptr(ptr), internal_deleter(std::move(deleter)) {}

    ~unique_ptr() {
        if (internal_ptr) {
            internal_deleter(internal_ptr);
        }
    }

    unique_ptr(const unique_ptr&) = delete;

    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& other) {
        internal_ptr = other.internal_ptr;
        other.internal_ptr = nullptr;
        internal_deleter = std::move(other.internal_deleter);
    }

    unique_ptr& operator=(unique_ptr&& other) {
        if (this == &other) {
            return *this;
        }
        internal_deleter(internal_ptr);
        internal_ptr = other.internal_ptr;
        other.internal_ptr = nullptr;
        internal_deleter = std::move(other.internal_deleter);
        return *this;
    }

    T& operator*() const {
        return *internal_ptr;
    }
    
    T* operator->() const {
        return internal_ptr;
    }

    T* get() const {
        return internal_ptr;
    }

    explicit operator bool() const { // prevent bool from converting to 0 1
        return internal_ptr != nullptr;
    }

    D& get_deleter() {
        return internal_deleter;
    }

    const D& get_deleter() const {
        return internal_deleter;
    }

    T* release() {
        T* tmp = internal_ptr;
        internal_ptr = nullptr;
        return tmp;
    }

    void reset(T* ptr = nullptr) { 
        if (internal_ptr) {
            internal_deleter(internal_ptr);
        }
        internal_ptr = ptr;
    }

};
