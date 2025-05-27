/*
    MIT License

    Copyright (c) 2025 OpenPRA Initiative
    Copyright (c) 2025 Arjun Earthperson

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

#include "mef/openpsa/event/event.h"
namespace mef::openpsa {
/// A representation of a gate in a fault tree.
class Gate : public Event, public NodeMark {
  public:
    static constexpr const char *kTypeString = "gate"; ///< Type for errors only.

    using Event::Event;

    /// @returns true if the gate formula has been set.
    [[nodiscard]] bool HasFormula() const { return formula_ != nullptr; }

    /// @returns The formula of this gate.
    ///
    /// @pre The gate has its formula initialized.
    ///
    /// @{
    [[nodiscard]] const Formula &formula() const {
        assert(formula_ && "Gate formula is not set.");
        return *formula_;
    }
    Formula &formula() { return const_cast<Formula &>(std::as_const(*this).formula()); }
    /// @}

    /// Sets the formula of this gate.
    ///
    /// @param[in] formula  The new Boolean formula of this gate.
    ///
    /// @returns The old formula.
    std::unique_ptr<Formula> formula(std::unique_ptr<Formula> formula) {
        assert(formula && "Cannot unset formula.");
        formula_.swap(formula);
        return formula;
    }

  private:
    std::unique_ptr<Formula> formula_; ///< Boolean formula of this gate.
};
}