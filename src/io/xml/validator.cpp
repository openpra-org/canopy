/**
 * @file validator.cpp
 * @brief Implements the canopy::io::xml::validator class for RelaxNG XML validation.
 *
 * This file contains the implementation of the validator class, which loads a RelaxNG schema
 * and validates XML documents against it using libxml2.
 */

#include "io/xml/validator.h"

namespace io::xml {

/**
 * @brief Constructs a validator by loading and parsing a RelaxNG schema file.
 *
 * This constructor initializes the libxml2 parser context, parses the schema, and prepares
 * a validation context for subsequent document validation. It throws an exception if any
 * step fails, ensuring that only a valid and ready-to-use validator object is constructed.
 *
 * @param rng_file Path to the RelaxNG schema file.
 * @throws canopy::io::xml::error if the schema cannot be loaded, parsed, or the context cannot be created.
 *
 * @see validator::validate
 */
validator::validator(const std::string &rng_file) {
    // Clear any previous libxml2 errors to ensure clean error reporting for this operation.
    xmlResetLastError();

    // Create a parser context for the RelaxNG schema file.
    // The unique_ptr ensures proper cleanup of the parser context.
    const std::unique_ptr<xmlRelaxNGParserCtxt, decltype(&xmlRelaxNGFreeParserCtxt)> parser_ctxt(
        xmlRelaxNGNewParserCtxt(rng_file.c_str()), &xmlRelaxNGFreeParserCtxt);

    if (!parser_ctxt) {
        // Parser context creation failed, likely due to logic or resource issues.
        throw error(error_type::logic);
    }

    // Parse the RelaxNG schema from the parser context.
    schema_.reset(xmlRelaxNGParse(parser_ctxt.get()));
    if (!schema_) {
        // Schema parsing failed, possibly due to syntax errors in the schema file.
        throw error(error_type::parse);
    }

    // Create a validation context for the parsed schema.
    ctxt_.reset(xmlRelaxNGNewValidCtxt(schema_.get()));
    if (!ctxt_) {
        // Validation context creation failed, which is a logic error.
        throw error(error_type::logic);
    }
}

/**
 * @brief Validates an XML document against the loaded RelaxNG schema.
 *
 * This method checks the provided document for conformance to the schema. If the document
 * is invalid, an exception is thrown. The method resets libxml2's error state before validation
 * to ensure accurate error reporting.
 *
 * @param doc The XML document to validate.
 * @throws canopy::io::xml::error if the document does not conform to the schema.
 *
 * @see validator::validator
 */
void validator::validate(const document &doc) const {
    // Clear any previous libxml2 errors to ensure clean error reporting for this operation.
    xmlResetLastError();

    // Validate the document against the schema.
    // Note: xmlRelaxNGValidateDoc expects a non-const xmlDoc*, but does not modify the document.
    if (xmlRelaxNGValidateDoc(ctxt_.get(), const_cast<xmlDoc *>(doc.get())) != 0) {
        // Validation failed; throw an exception to signal invalidity.
        throw error(error_type::validity);
    }
}

} // namespace canopy::io::xml