#include "io/xml/validator.h"

namespace canopy::io::xml {

    validator::validator(const std::string& rng_file) {
        xmlResetLastError();
        const std::unique_ptr<xmlRelaxNGParserCtxt, decltype(&xmlRelaxNGFreeParserCtxt)>
            parser_ctxt(xmlRelaxNGNewParserCtxt(rng_file.c_str()), &xmlRelaxNGFreeParserCtxt);
        if (!parser_ctxt) {
            throw error(xml::error_type::logic);
        }
        schema_.reset(xmlRelaxNGParse(parser_ctxt.get()));
        if (!schema_) {
            throw error(xml::error_type::parse);
        }
        ctxt_.reset(xmlRelaxNGNewValidCtxt(schema_.get()));
        if (!ctxt_) {
            throw error(xml::error_type::logic);
        }
    }

    void validator::validate(const document& doc) const {
        xmlResetLastError();
        if (xmlRelaxNGValidateDoc(ctxt_.get(), const_cast<xmlDoc*>(doc.get())) != 0) {
            throw error(xml::error_type::validity);
        }
    }

} // namespace canopy::io::xml