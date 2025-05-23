#include "io/xml/range.h"
#include "io/xml/element.h"

#include <cassert>
#include <iterator>

namespace canopy::io::xml {

    //────────────────────────────────────────────────────────────────────────────
    // range::iterator implementation
    //────────────────────────────────────────────────────────────────────────────

    range::iterator::iterator(const xmlElement* elem)
    : element_(elem)
    {
    }

    range::iterator& range::iterator::operator++() {
        assert(element_ && "Incrementing end iterator!");
        element_ = find_element(element_->next);
        return *this;
    }

    range::iterator range::iterator::operator++(int) {
        iterator temp(*this);
        ++(*this);
        return temp;
    }

    bool range::iterator::operator==(const iterator& other) const {
        return element_ == other.element_;
    }

    range::iterator::value_type range::iterator::operator*() const {
        // Construct an element from the current xmlElement*
        return element(element_);
    }

    // static
    const xmlElement* range::iterator::find_element(const xmlNode* node) noexcept {
        while (node && node->type != XML_ELEMENT_NODE) {
            node = node->next;
        }
        return reinterpret_cast<const xmlElement*>(node);
    }

    //────────────────────────────────────────────────────────────────────────────
    // range implementation
    //────────────────────────────────────────────────────────────────────────────

    range::range(const xmlNode* head)
    : begin_(find_element(head))
    {
    }

    range::iterator range::begin() const {
        return iterator(begin_);
    }

    range::iterator range::end() const {
        return iterator(nullptr);
    }

    // static
    const xmlElement* range::find_element(const xmlNode* node) noexcept {
        while (node && node->type != XML_ELEMENT_NODE) {
            node = node->next;
        }
        return reinterpret_cast<const xmlElement*>(node);
    }

    bool range::empty() const {
        return begin() == end();
    }

    std::size_t range::size() const {
        return static_cast<std::size_t>(std::distance(begin(), end()));
    }

} // namespace canopy::io::xml