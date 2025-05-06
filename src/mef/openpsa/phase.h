#pragma once

#include <vector>

namespace mef::openpsa {
/// Phases of alignments the models spends its time fraction.
class Phase : public Element {
  public:
    static constexpr const char* kTypeString = "phase";  ///< For error messages.

    /// @copydoc Element::Element
    ///
    /// @param[in] time_fraction  The fraction of mission-time spent in the phase.
    ///
    /// @throws DomainError  The fraction is not a valid value in (0, 1].
    Phase(std::string name, double time_fraction)     : Element(std::move(name)), time_fraction_(time_fraction) {
        if (time_fraction_ <= 0 || time_fraction_ > 1)
            throw(DomainError("The phase fraction must be in (0, 1]: "+(std::to_string(time_fraction_) + "element: "+Element::name() + " " +kTypeString)));
    }

    /// @returns The positive fraction of mission-time spent in this phase.
    double time_fraction() const { return time_fraction_; }

    /// @returns The instructions applied in this phase.
    const std::vector<SetHouseEvent*>& instructions() const {
        return instructions_;
    }

    /// @param[in] instructions  Zero or more instructions for this phase.
    void instructions(std::vector<SetHouseEvent*> instructions) {
        instructions_ = std::move(instructions);
    }

  public:
    double time_fraction_;  ///< The positive fraction of the mission time.
    std::vector<SetHouseEvent*> instructions_;  ///< The phase modifiers.
};
}