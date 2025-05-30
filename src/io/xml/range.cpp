/**
 * @file range.cpp
 * @brief Implementation of canopy::io::xml::range and range::iterator.
 *
 * This file provides the implementation for the range and range::iterator classes,
 * which allow for forward iteration over a linked list of XML element nodes,
 * skipping non-element nodes in the underlying libxml2 structure.
 */

#include "io/xml/range.h"
#include "io/xml/element.h"
#include <cassert>
#include <iterator>

namespace canopy::io::xml {

// ────────────────────────────────────────────────────────────────────────────
//  range::iterator implementation
// ────────────────────────────────────────────────────────────────────────────

/**
 * @brief Constructs an iterator at the given XML element node.
 * @param elem Pointer to the starting xmlElement node, or nullptr for end.
 */
range::iterator::iterator(const xmlElement *elem) : element_(elem) {}

/**
 * @brief Advances the iterator to the next XML element node (pre-increment).
 * @return Reference to this iterator.
 * @throws Assertion failure if incrementing past the end.
 *
 * Example:
 * @code
 * ++it;
 * @endcode
 */
range::iterator &range::iterator::operator++() {
    assert(element_ && "Incrementing end iterator!");
    element_ = find_element(element_->next);
    return *this;
}

/**
 * @brief Advances the iterator to the next XML element node (post-increment).
 * @return Copy of the iterator before increment.
 *
 * Example:
 * @code
 * it++;
 * @endcode
 */
range::iterator range::iterator::operator++(int) {
    const iterator temp(*this);
    ++*this;
    return temp;
}

/**
 * @brief Compares two iterators for equality.
 * @param other The iterator to compare with.
 * @return True if both iterators point to the same element.
 */
bool range::iterator::operator==(const iterator &other) const { return element_ == other.element_; }

/**
 * @brief Dereferences the iterator to access the current element.
 * @return The current element (by value).
 *
 * Example:
 * @code
 * auto elem = *it;
 * @endcode
 */
range::iterator::value_type range::iterator::operator*() const {
    // Construct an element from the current xmlElement*
    return element(element_);
}

/**
 * @brief Finds the first XML element node in a linked list of xmlNode.
 * @param node The starting node.
 * @return Pointer to the first xmlElement node, or nullptr if none found.
 *
 * This helper skips non-element nodes, returning the first element node or nullptr.
 */
const xmlElement *range::iterator::find_element(const xmlNode *node) noexcept {
    while (node && node->type != XML_ELEMENT_NODE) {
        node = node->next;
    }
    return reinterpret_cast<const xmlElement *>(node);
}

// ────────────────────────────────────────────────────────────────────────────
//  range implementation
// ────────────────────────────────────────────────────────────────────────────

/**
 * @brief Constructs a range over an intrusive list of XML element nodes.
 * @param head The head of the list (may be a non-element node, or nullptr if empty).
 *
 * Example:
 * @code
 * canopy::io::xml::range children(parent_node->children);
 * @endcode
 */
range::range(const xmlNode *head) : begin_(find_element(head)) {}

/**
 * @brief Returns an iterator to the first XML element node in the range.
 * @return Iterator to the beginning.
 */
range::iterator range::begin() const { return iterator(begin_); }

/**
 * @brief Returns an iterator to one-past-the-last XML element node in the range.
 * @return Iterator to the end.
 */
range::iterator range::end() { return iterator(nullptr); }

/**
 * @brief Helper to skip non-element nodes at construction time.
 * @param node The starting node.
 * @return Pointer to the first xmlElement node, or nullptr if none found.
 */
const xmlElement *range::find_element(const xmlNode *node) noexcept {
    while (node && node->type != XML_ELEMENT_NODE) {
        node = node->next;
    }
    return reinterpret_cast<const xmlElement *>(node);
}

/**
 * @brief Checks if the range contains no XML element nodes.
 * @return True if the range is empty.
 *
 * Example:
 * @code
 * if (children.empty()) { ... }
 * @endcode
 */
bool range::empty() const { return begin() == end(); }

/**
 * @brief Returns the number of XML element nodes in the range.
 * @return The count of elements (O(N) complexity).
 *
 * Example:
 * @code
 * std::size_t n = children.size();
 * @endcode
 */
std::size_t range::size() const { return static_cast<std::size_t>(std::distance(begin(), end())); }

} // namespace canopy::io::xml