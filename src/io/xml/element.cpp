#include "io/xml/element.h"

namespace canopy::io::xml {

    element::element(const xmlElement* elem)
    : element_(elem)
    {
        assert(element_);
    }

    const char* element::filename() const {
        // Provided by libxml, URL is the document's URI.
        return helpers::from_utf8(element_->doc->URL);
    }

    int element::line() const {
        return XML_GET_LINE(to_node());
    }

    std::string_view element::name() const {
        return helpers::from_utf8(element_->name);
    }

    bool element::has_attribute(const char* name) const {
        return (xmlHasProp(to_node(), helpers::to_utf8(name)) != nullptr);
    }

    std::string_view element::attribute(const char* name) const {
        const xmlAttr* property = xmlHasProp(to_node(), helpers::to_utf8(name));
        if (!property) {
            return {};
        }
        const xmlNode* text_node = property->children;
        assert(text_node && text_node->type == XML_TEXT_NODE);
        assert(text_node->content);
        return helpers::trim(helpers::from_utf8(text_node->content));
    }

    std::string_view element::text() const {
        const xmlNode* text_node = element_->children;
        while (text_node && text_node->type != XML_TEXT_NODE) {
            text_node = text_node->next;
        }
        assert(text_node && "element does not have text.");
        assert(text_node->content && "Missing text in element.");
        return helpers::trim(helpers::from_utf8(text_node->content));
    }

    std::optional<element> element::child(std::string_view name) const {
        for (auto e : children()) {
            if (name.empty() || name == e.name()) {
                return e;
            }
        }
        return {};
    }

    range element::children() const {
        return range(element_->children);
    }

    xmlNode* element::to_node() const {
        // Downcast xmlElement* -> xmlNode*
        return reinterpret_cast<xmlNode*>(const_cast<xmlElement*>(element_));
    }

} // namespace canopy::io::xml