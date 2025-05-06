/// @file
/// XML facility expensive wrappers implemented out-of-line.

#include "io/xml/xml.h"

#include <libxml/xinclude.h>

namespace io::xml {

[[maybe_unused]] Document::Document(const std::string& file_path, Validator* validator)
   : doc_(nullptr, &xmlFreeDoc) {
   xmlResetLastError();
   doc_.reset(xmlReadFile(file_path.c_str(), nullptr, kParserOptions));
   xmlErrorPtr xml_error = xmlGetLastError();
   if (xml_error) {
       if (xml_error->domain == xmlErrorDomain::XML_FROM_IO) {
           throw IOError(xml_error->message, file_path, errno, "r");
       }
       throw detail::GetError<ParseError>(xml_error);
   }
   assert(doc_ && "Internal XML library failure.");
   if (xmlXIncludeProcessFlags(get(), kParserOptions) < 0 || xmlGetLastError()) {
       throw detail::GetError<XIncludeError>();
   }
   if (validator) {
       validator->validate(*this);
   }
}

[[maybe_unused]] Validator::Validator(const std::string& rng_file)
   : schema_(nullptr, &xmlRelaxNGFree),
     valid_ctxt_(nullptr, &xmlRelaxNGFreeValidCtxt) {
   xmlResetLastError();
   std::unique_ptr<xmlRelaxNGParserCtxt, decltype(&xmlRelaxNGFreeParserCtxt)>
       parser_ctxt(xmlRelaxNGNewParserCtxt(rng_file.c_str()),
                   &xmlRelaxNGFreeParserCtxt);
   if (!parser_ctxt) {
       throw detail::GetError<LogicError>();
   }
   schema_.reset(xmlRelaxNGParse(parser_ctxt.get()));
   if (!schema_) {
       throw(detail::GetError<ParseError>());
   }
   valid_ctxt_.reset(xmlRelaxNGNewValidCtxt(schema_.get()));
   if (!valid_ctxt_) {
       throw(detail::GetError<LogicError>());
   }
}

}  // namespace canopy::io::xml
