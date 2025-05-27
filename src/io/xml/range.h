#pragma once

#include <iterator>
#include <libxml/tree.h>

namespace canopy::io::xml {

class element;

/// A simple view adaptor over a linked list of XML elements.
class range {
public:
using value_type = element;

/// Forward iterator over XML elements.
class iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = element;
    using reference         = element;
    using pointer           = element*;

    /// @param[in] elem The starting element in the list.
    /// nullptr signifies the end.
    explicit iterator(const xmlElement* elem = nullptr);

    // Increment (pre and post).
    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const { return !(*this == other); }

    /// @return Current element (by value).
    value_type operator*() const;

private:
    /// Find the first XML element node in a linked list of xmlNode.
    static const xmlElement* find_element(const xmlNode* node) noexcept;

    const xmlElement* element_ = nullptr;
};

using const_iterator = iterator;  // This range is immutable in practice.

/// Constructs the range over an intrusive list of XML element nodes.
///
/// @param[in] head The head of the list (may be a non-element node).
///                 nullptr if empty.
explicit range(const xmlNode* head);

// Standard range accessors.
[[nodiscard]] iterator begin() const;
[[nodiscard]] iterator end() const;
[[nodiscard]] iterator cbegin() const { return begin(); }
[[nodiscard]] iterator cend() const { return end(); }

/// @return true if there are no XML elements in this range.
[[nodiscard]] bool empty() const;

/// @return The count of elements in this range (O(N) complexity).
[[nodiscard]] std::size_t size() const;
private:
// Helper to skip non-element nodes at construction time.
static const xmlElement* find_element(const xmlNode* node) noexcept;

const xmlElement* begin_{nullptr};
};

} // namespace canopy::io::xml
