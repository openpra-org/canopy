/// @file
/// Mission and phase constructs.

#pragma once

#include <string>
#include <vector>

#include "mef/openpsa/algorithm.h"
#include "mef/openpsa/element.h"
#include "mef/openpsa/instruction.h"
#include "mef/openpsa/phase.h"

namespace mef::openpsa {

/// Alignment configuration for the whole model per analysis.
class Alignment : public Element, public Container<Alignment, Phase> {
 public:
   /// Type string for error messages.
   static constexpr const char* kTypeString = "alignment";

   using Element::Element;

   /// @returns The table range of phases in the alignment.
   [[nodiscard]] auto phases() const { return Container::table(); }

   /// Ensures that all phases add up to be valid for the alignment.
   ///
   /// @throws ValidityError  Phases are incomplete (e.g., don't sum to 1).
   [[maybe_unused]] void Validate() {
       double sum = 0;
       for (const Phase& phase : phases())
           sum += phase.time_fraction();
       if (!is_close(1, sum, 1e-4))
           throw(ValidityError("The phases of the alignment do not sum to 1: "+std::to_string(sum) + Element::name()));
   }
};

}  // namespace scram::mef
