#pragma once

/**
 * @file range.h
 * @brief Defines canopy::io::xml::range, a view adaptor for iterating over XML element nodes.
 *
 * This file provides the range and range::iterator classes, which allow for convenient, STL-style
 * iteration over a linked list of XML element nodes (skipping non-element nodes).
 */

#include <iterator>
#include <libxml/tree.h>

namespace io::xml {

class element;

/**
 * @class range
 * @brief A simple, immutable view adaptor over a linked list of XML element nodes.
 *
 * The range class provides a forward-iterable view over a sequence of XML element nodes,
 * skipping any non-element nodes in the underlying libxml2 linked list.
 *
 * Example usage:
 * @code
 * canopy::io::xml::range children(some_xmlNode->children);
 * for (const auto& elem : children) {
 *     std::cout << elem.name() << std::endl;
 * }
 * @endcode
 */
class range {
  public:
    using value_type = element;

    /**
     * @class iterator
     * @brief Forward iterator over XML element nodes.
     *
     * This iterator skips non-element nodes and provides value access to each element.
     * It is a lightweight, single-pass iterator.
     *
     * Example usage:
     * @code
     * for (range::iterator it = children.begin(); it != children.end(); ++it) {
     *     std::cout << (*it).name() << std::endl;
     * }
     * @endcode
     */
    class iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = element;
        using reference = element;
        using pointer = element *;

        /**
         * @brief Constructs an iterator at the given XML element node.
         * @param[in] elem The starting element in the list. nullptr signifies the end.
         */
        explicit iterator(const xmlElement *elem = nullptr);

        /**
         * @brief Advances the iterator to the next XML element node (pre-increment).
         * @return Reference to this iterator.
         * @throws Assertion failure if incrementing past the end.
         */
        iterator &operator++();

        /**
         * @brief Advances the iterator to the next XML element node (post-increment).
         * @return Copy of the iterator before increment.
         */
        iterator operator++(int);

        /**
         * @brief Compares two iterators for equality.
         * @param other The iterator to compare with.
         * @return True if both iterators point to the same element.
         */
        bool operator==(const iterator &other) const;

        /**
         * @brief Compares two iterators for inequality.
         * @param other The iterator to compare with.
         * @return True if the iterators point to different elements.
         */
        bool operator!=(const iterator &other) const { return !(*this == other); }

        /**
         * @brief Dereferences the iterator to access the current element.
         * @return The current element (by value).
         */
        value_type operator*() const;

      private:
        /**
         * @brief Finds the first XML element node in a linked list of xmlNode.
         * @param node The starting node.
         * @return Pointer to the first xmlElement node, or nullptr if none found.
         */
        static const xmlElement *find_element(const xmlNode *node) noexcept;

        /// Pointer to the current XML element node (nullptr if at end).
        const xmlElement *element_ = nullptr;
    };

    /// @brief Alias for const_iterator (range is immutable).
    using const_iterator = iterator;

    /**
     * @brief Constructs a range over an intrusive list of XML element nodes.
     * @param[in] head The head of the list (may be a non-element node, or nullptr if empty).
     *
     * Example:
     * @code
     * canopy::io::xml::range children(parent_node->children);
     * @endcode
     */
    explicit range(const xmlNode *head);

    /**
     * @brief Returns an iterator to the first XML element node in the range.
     * @return Iterator to the beginning.
     */
    [[nodiscard]] iterator begin() const;

    /**
     * @brief Returns an iterator to one-past-the-last XML element node in the range.
     * @return Iterator to the end.
     */
    [[nodiscard]] static iterator end() ;

    /**
     * @brief Returns a const iterator to the beginning of the range.
     * @return Const iterator to the beginning.
     */
    [[nodiscard]] iterator cbegin() const { return begin(); }

    /**
     * @brief Returns a const iterator to the end of the range.
     * @return Const iterator to the end.
     */
    [[nodiscard]] static iterator cend() { return end(); }

    /**
     * @brief Checks if the range contains no XML element nodes.
     * @return True if the range is empty.
     */
    [[nodiscard]] bool empty() const;

    /**
     * @brief Returns the number of XML element nodes in the range.
     * @return The count of elements (O(N) complexity).
     */
    [[nodiscard]] std::size_t size() const;

  private:
    /**
     * @brief Helper to skip non-element nodes at construction time.
     * @param node The starting node.
     * @return Pointer to the first xmlElement node, or nullptr if none found.
     */
    static const xmlElement *find_element(const xmlNode *node) noexcept;

    /// Pointer to the first XML element node in the range (nullptr if empty).
    const xmlElement *begin_{nullptr};
};

} // namespace canopy::io::xml
