#ifndef BOOLEXPR_UTIL_ARENA_H
#define BOOLEXPR_UTIL_ARENA_H

#include <cstddef>
// #include <limits>
#include <optional>
#include <span>
#include <stack>
#include <vector>
#include <numeric>

#include "./helper_concepts.h"


namespace canopy::boolexpr::util {

template <class T, UnsignedInteger IndexType = std::size_t> class ObjectArena {
  public:
    using idx_type = IndexType;
    using value_type = T;

    // constexpr static auto NULL_IDX = std::numeric_limits<idx_type>::max();

    explicit ObjectArena(const std::size_t capacity, const bool reserve = true) : capacity_{capacity} {
        if (not capacity_) {
            return;
        }

        if (reserve) {
            storage_.reserve(capacity);
            occupied_.reserve(capacity);
        }

        free_list_.push(idx_type{0});
    }

    ~ObjectArena() = default;

    // Copy and move operations are deleted for simplicity and clear ownership.
    ObjectArena(const ObjectArena &) = delete;
    ObjectArena &operator=(const ObjectArena &) = delete;
    ObjectArena(ObjectArena &&) = delete;
    ObjectArena &operator=(ObjectArena &&) = delete;

    template <class... Args>
    [[nodiscard]]
    std::optional<idx_type> create(Args &&...args) {
        if (free_list_.empty()) {
            // return NULL_IDX;
            return std::nullopt;
        }

        const auto idx{free_list_.top()};

        if (idx == this->size()) { // Place the new obj at the end of the storage
            storage_.emplace_back(std::forward<Args>(args)...);
            occupied_.emplace_back(true);

            free_list_.pop();

            if (storage_.size() != capacity_) {
                free_list_.push(storage_.size());
            }
        } else { // Place the new object in a place of previously deallocated one
            storage_.at(idx) = T{std::forward<Args>(args)...};
            occupied_.at(idx) = true;
            free_list_.pop();
        }

        return idx;
    }

    bool destroy(const idx_type idx) {
        if (idx >= storage_.size() || !occupied_.at(idx)) {
            return false;
        }
        occupied_.at(idx) = false;
        free_list_.push(idx);
        return true;
    }

    T* at(const idx_type idx) {
        if (this->occupied(idx)) {
            return &storage_.at(idx);
        }
        return nullptr;
    }

    const T *at(const idx_type idx) const {
        if (this->occupied(idx)) {
            return &storage_.at(idx);
        }
        return nullptr;
    }

    constexpr bool occupied(const idx_type idx) const {
        return idx < storage_.size() && occupied_.at(idx);
    }

    [[nodiscard]]
    constexpr std::size_t size() const noexcept { return storage_.size(); }

    [[nodiscard]]
    constexpr std::size_t capacity() const noexcept { return capacity_; }

    [[nodiscard]]
    constexpr std::size_t storage_capacity() const noexcept { return storage_.capacity(); }

    [[nodiscard]]
    constexpr std::size_t count() const noexcept {
        return std::reduce(occupied_.cbegin(), occupied_.cend(), std::size_t{});
    }

  private:
    std::size_t capacity_{};
    std::vector<value_type> storage_{};
    std::vector<bool> occupied_{};
    std::stack<idx_type, std::vector<idx_type>> free_list_{};
};


template <class T, UnsignedInteger IndexType = std::size_t> class SetArena {
  public:
    using value_type = T;
    using index_type = IndexType;

    // constexpr static auto NULL_IDX = std::numeric_limits<size_type>::max();

    explicit SetArena(const std::size_t set_capacity, const std::size_t element_capacity, const bool reserve = true)
        : capacity_{element_capacity}
    {
        if (not capacity_) {
            return;
        }

        if (reserve) {
            storage_.reserve(capacity_);
            indptr_.reserve(set_capacity + 1U);
        }

        indptr_.emplace_back(storage_.size());
    }

    std::optional<index_type> store(std::span<T> values) {
        if (storage_.size() + values.size() > capacity_) {
            return std::nullopt;
        }

        std::copy(
            std::begin(values),
            std::end(values),
            std::back_inserter(storage_)
        );

        const auto new_idx{indptr_.size() - 1U};
        indptr_.emplace_back(storage_.size());

        return new_idx;
    }

    std::span<const T> get(const index_type idx) const {
        if (idx >= indptr_.size() - 1U) {
            return {};
        }

        const auto start = storage_.cbegin();
        return {start + indptr_.at(idx), start + indptr_.at(idx + 1U),};
    }

    [[nodiscard]]
    std::string to_string() const {
        std::string result{};

        for (std::size_t i = 0; i < indptr_.size() - 1U; ++i) {
            result += fmt::format("{}: {}\n", i, get(i));
        }

        return result;
    }

    // TODO implement "add elements to the last set"

  private:
    std::size_t capacity_{};
    std::vector<T> storage_;
    std::vector<IndexType> indptr_{};
};

} // namespace canopy::boolexpr::util

#endif
