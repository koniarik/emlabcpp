#include <new>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp {

// Data container for up to N elements, mirroring std::vector behavior.
template <typename T, std::size_t N>
class static_vector {

        // type for storage of one item
        using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

      public:
        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T &;
        using const_reference = const T &;
        using iterator        = T *;
        using const_iterator  = const T *;

        // public methods
        // --------------------------------------------------------------------------------
        static_vector() = default;
        static_vector(const static_vector &other) {
                for (size_type i = 0; i < other.size(); ++i) {
                        push_back(other[i]);
                }
        }
        static_vector(static_vector &&other) noexcept {
                for (size_type i = 0; i < other.size(); ++i) {
                        push_back(std::move(other[i]));
                }
        }
        static_vector &operator=(const static_vector &other) {
                if (this != &other) {
                        this->~static_vector();
                        ::new (this) static_vector(other);
                }
                return *this;
        }
        static_vector &operator=(static_vector &&other) noexcept {
                if (this != &other) {
                        this->~static_vector();
                        ::new (this) static_vector(std::move(other));
                }
                return *this;
        }

        // methods for handling the front side of the circular buffer
        
        [[nodiscard]] reference       front() { return ref_item(0); }
        [[nodiscard]] const_reference front() const { return ref_item(0); }

        // methods for handling the back side of the circular buffer
        
        void push_back(T item) { emplace_back(std::move(item)); }

        template <typename... Args>
        void emplace_back(Args &&... args) {
                emplace_item(size_, std::forward<Args>(args)...);
                size_ += 1;
        }

        T pop_back() {
                T item = std::move(back());
                delete_item(size_);
                size_ -= 1;
                return item;
        }

        [[nodiscard]] reference       back() { return ref_item(size_ - 1); }
        [[nodiscard]] const_reference back() const { return ref_item(size_ - 1); }
        
        // other methods

        [[nodiscard]] constexpr std::size_t max_size() const { return N; }

        [[nodiscard]] std::size_t size() const { return size_; }

        [[nodiscard]] bool empty() const { return size_ == 0; }

        [[nodiscard]] bool full() const { return size_ == N; }

        const_reference operator[](size_type i) const { return ref_item(i); }
        reference       operator[](size_type i) { return ref_item(i); }

        void clear() { purge(); }

        ~static_vector() { purge(); }

      private:
        // private attributes
        // --------------------------------------------------------------------------------

        storage_type data_[N] = {0}; // storage of the entire dataset
        size_type    size_    = 0;   // count of items

        // private methods
        // --------------------------------------------------------------------------------
        void delete_item(size_type i) { ref_item(i).~T(); }

        template <typename... Args>
        void emplace_item(size_type i, Args &&... args) {
                void *gen_ptr = reinterpret_cast<void *>(&data_[i]);
                ::new (gen_ptr) T(std::forward<Args>(args)...);
        }

        // Reference to the item in data_storage. std::launder is necessary here per the paper
        // linked above.
        [[nodiscard]] reference ref_item(size_type i) {
                return *std::launder(reinterpret_cast<T *>(&data_[i]));
        }
        [[nodiscard]] const_reference ref_item(size_type i) const {
                return *std::launder(reinterpret_cast<const T *>(&data_[i]));
        }

        // Cleans entire buffer from items.
        void purge() {
                while (!empty()) {
                        pop_back();
                }
        }
};

template <typename T, std::size_t N>
[[nodiscard]] inline bool operator==(const static_vector<T, N> &lh, const static_vector<T, N> &rh) {
        auto size = lh.size();
        if (size != rh.size()) {
                return false;
        }

        for (std::size_t i = 0; i < size; ++i) {
                if (lh[i] != rh[i]) {
                        return false;
                }
        }
        return true;
}

template <typename T, std::size_t N>
[[nodiscard]] inline bool operator!=(const static_vector<T, N> &lh, const static_vector<T, N> &rh) {
        return !(lh == rh);
}

} // namespace emlabcpp
