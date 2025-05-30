#pragma once

#include <memory>
#include <string>
#include <libxml/relaxng.h>

#include "io/xml/document.h"

namespace canopy::io::xml {

    class document;
    class validator {
    public:
        explicit validator(const std::string& rng_file);
        void validate(const document& doc) const;

    private:
        std::unique_ptr<xmlRelaxNG, decltype(&xmlRelaxNGFree)> schema_{nullptr, &xmlRelaxNGFree};
        std::unique_ptr<xmlRelaxNGValidCtxt, decltype(&xmlRelaxNGFreeValidCtxt)> ctxt_{nullptr, &xmlRelaxNGFreeValidCtxt};
    };

} // namespace canopy::io::xml