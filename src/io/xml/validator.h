#pragma once

/**
 * @file validator.h
 * @brief Defines the canopy::io::xml::validator class for RelaxNG XML validation.
 *
 * This file provides the interface for the validator class, which encapsulates RelaxNG schema loading
 * and document validation using libxml2.
 */

#include "io/xml/document.h"
#include <libxml/relaxng.h>
#include <memory>
#include <string>

namespace canopy::io::xml {

class document;

/**
 * @class validator
 * @brief Validates XML documents against a RelaxNG schema.
 *
 * This class loads a RelaxNG schema from file and provides a method to validate XML documents
 * against it. It manages the lifetime of libxml2 validation objects.
 *
 * Typical usage:
 * @code
 * canopy::io::xml::validator v("schema.rng");
 * canopy::io::xml::document doc("file.xml");
 * v.validate(doc); // throws on validation error
 * @endcode
 */
class validator {
  public:
    /**
     * @brief Constructs a validator from a RelaxNG schema file.
     *
     * Loads and parses the RelaxNG schema, preparing it for validation.
     *
     * @param rng_file Path to the RelaxNG schema file.
     * @throws canopy::io::xml::error if the schema cannot be loaded or parsed.
     *
     * @code
     * canopy::io::xml::validator v("schema.rng");
     * @endcode
     */
    explicit validator(const std::string &rng_file);

    /**
     * @brief Validates an XML document against the loaded RelaxNG schema.
     *
     * Throws an exception if the document does not conform to the schema.
     *
     * @param doc The XML document to validate.
     * @throws canopy::io::xml::error if validation fails.
     *
     * @code
     * v.validate(doc); // throws if doc is invalid
     * @endcode
     */
    void validate(const document &doc) const;

  private:
    /**
     * @brief Smart pointer managing the RelaxNG schema object.
     *
     * Uses a custom deleter (`xmlRelaxNGFree`) for proper cleanup.
     */
    std::unique_ptr<xmlRelaxNG, decltype(&xmlRelaxNGFree)> schema_{nullptr, &xmlRelaxNGFree};

    /**
     * @brief Smart pointer managing the RelaxNG validation context.
     *
     * Uses a custom deleter (`xmlRelaxNGFreeValidCtxt`) for proper cleanup.
     */
    std::unique_ptr<xmlRelaxNGValidCtxt, decltype(&xmlRelaxNGFreeValidCtxt)> ctxt_{nullptr, &xmlRelaxNGFreeValidCtxt};
};

} // namespace canopy::io::xml