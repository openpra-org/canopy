#include "io/xml/document.h"

#include <filesystem>
#include <libxml/xinclude.h>

namespace canopy::io::xml {

    document::document(const std::string& file_path, validator* validator) {
        xmlResetLastError();
        doc_.reset(xmlReadFile(file_path.c_str(), nullptr, parser_options_));

        if (const xmlErrorPtr xml_error = xmlGetLastError()) {
            if (xml_error->domain == XML_FROM_IO) {
                const std::string msg = std::string(xml_error->message) + file_path + std::to_string(errno);
                throw error(error_type::io, xml_error, msg);
            }
            throw error(error_type::parse, xml_error);
        }

        assert(doc_ && "Internal XML library failure.");

        if (xmlXIncludeProcessFlags(get(), parser_options_) < 0 || xmlGetLastError()) {
            throw error(error_type::x_include);
        }

        if (validator) {
            validator->validate(*this);
        }
    }

    element document::root() const noexcept {
        return element(
            reinterpret_cast<const xmlElement*>(xmlDocGetRootElement(doc_.get()))
        );
    }

} // namespace canopy::io::xml