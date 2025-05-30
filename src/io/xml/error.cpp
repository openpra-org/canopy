#include "io/xml/error.h"

#include <format>

namespace canopy::io::xml {

    error::error(const error_type code)
        : code_{code}
    {
        msg_ = "";
    }

    error::error(const error_type code, const std::string &msg)
        : code_{code}, msg_{format_message(code, msg)}
    {
    }



    error::error(const error_type code, xmlErrorPtr xml_error)
    : code_{code}
    {
        msg_ = "";
        if (!xml_error)
            xml_error = xmlGetLastError();
        if (!xml_error) {
            msg_ += " (No XML error is available.)";
        } else {
            if (xml_error->file)
                msg_ += " " + std::string(xml_error->file);
            if (xml_error->line)
                msg_ += " line " + std::to_string(xml_error->line);
        }
        msg_ = format_message(code, msg_);
    }

    error::error(const error_type code, xmlErrorPtr xml_error, const std::string &msg)
        : code_{code}
    {
        msg_ = msg;
        if (!xml_error)
            xml_error = xmlGetLastError();
        if (!xml_error) {
            msg_ += " (No XML error is available.)";
        } else {
            if (xml_error->file)
                msg_ += " " + std::string(xml_error->file);
            if (xml_error->line)
                msg_ += " line " + std::to_string(xml_error->line);
        }
        msg_ = format_message(code, msg_);
    }

    std::string error::format_message(const error_type code, const std::string& msg) {
        return std::format("{}: {}", to_string(code), msg);
    }

} // namespace canopy::io::xml