#pragma once

#include <memory>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "io/xml/validator.h"
#include "io/xml/element.h"

namespace canopy::io::xml {

    class validator;

    class document {
    public:
        // Parse an XML file and resolve XInclude
        explicit document(const std::string& file_path, validator* validator = nullptr);

        // root element accessor
        [[nodiscard]] element root() const noexcept;

        // give low-level access for validators etc.
        [[nodiscard]] const xmlDoc* get() const noexcept { return doc_.get(); }
        [[nodiscard]] xmlDoc* get() noexcept { return doc_.get(); }

        static constexpr int parser_options_ =
            XML_PARSE_XINCLUDE | XML_PARSE_NOBASEFIX | XML_PARSE_NONET |
            XML_PARSE_NOXINCNODE | XML_PARSE_COMPACT | XML_PARSE_HUGE;

    private:
        std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> doc_{nullptr, &xmlFreeDoc};
    };

} // namespace canopy::io::xml