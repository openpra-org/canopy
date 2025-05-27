#pragma once

namespace mef::openpsa {
/// Boolean formula with connectives and arguments.
/// Formulas are not expected to be shared.
class Formula {
  public:
    /// Argument events of a formula.
    using ArgEvent = std::variant<Gate*, BasicEvent*, HouseEvent*>;

    /// Formula argument with a complement flag.
    struct Arg {
        bool complement;  ///< Negation of the argument event.
        ArgEvent event;  ///< The event in the formula.
    };

    /// The set of formula arguments.
    class ArgSet {
      public:
        /// Default constructor of an empty argument set.
        ArgSet() = default;

        /// Constructors from initializer lists and iterator ranges of args.
        /// @{
        ArgSet(std::initializer_list<Arg> init_list)
            : ArgSet(init_list.begin(), init_list.end()) {}

        ArgSet(std::initializer_list<ArgEvent> init_list)
            : ArgSet(init_list.begin(), init_list.end()) {}

        template <typename Iterator>
        ArgSet(Iterator first1, Iterator last1) {
            for (; first1 != last1; ++first1)
                Add(*first1);
        }
        /// @}

        /// Adds an event into the arguments set.
        ///
        /// @param[in] event  An argument event.
        /// @param[in] complement  Indicate the negation of the argument event.
        ///
        /// @throws DuplicateElementError  The argument event is duplicate.
        void Add(ArgEvent event, bool complement = false);

        /// Overload to add formula argument with a structure.
        void Add(Arg arg) { Add(arg.event, arg.complement); }

        /// Removes an event from the formula.
        ///
        /// @param[in] event  The argument event of this formula.
        ///
        /// @throws LogicError  The argument is not in the set.
        [[maybe_unused]] void Remove(ArgEvent event);

        /// @returns The underlying container with the data.
        /// @{
        const std::vector<Arg>& data() const { return args_; }
        std::vector<Arg>& data() { return args_; }
        /// @}

        /// @returns The number of arguments in the set.
        std::size_t size() const { return args_.size(); }

        /// @return true if the set is empty.
        bool empty() const { return args_.empty(); }

      private:
        std::vector<Arg> args_;  ///< The underlying data container.
    };

    /// @param[in] connective  The logical connective for this Boolean formula.
    /// @param[in] args  The arguments of the formula.
    /// @param[in] min_number  The min number relevant to the connective.
    /// @param[in] max_number  The max number relevant to the connective.
    ///
    /// @throws ValidityError  Invalid arguments or setup for the connective.
    /// @throws LogicError  Invalid nesting of complement or constant args.
    /// @throws LogicError  Negative values for min or max number.
    Formula(Connective connective, ArgSet args,
            std::optional<int> min_number = {},
            std::optional<int> max_number = {});

    /// Copy semantics only.
    /// @{
    Formula(const Formula&) = default;
    Formula& operator=(const Formula&) = default;
    /// @}

    /// @returns The connective of this formula.
    [[maybe_unused]] [[nodiscard]] Connective connective() const { return connective_; }

    /// @returns The min number for "atleast"/"cardinality" connective.
    [[maybe_unused]] [[nodiscard]] std::optional<int> min_number() const;

    /// @returns The max number of "cardinality" connective.
    [[maybe_unused]] std::optional<int> max_number() const;

    /// @returns The arguments of this formula.
    [[nodiscard]] const std::vector<Arg>& args() const { return args_.data(); }

    /// Swaps an argument event with another one.
    ///
    /// @param[in] current  The current argument event in this formula.
    /// @param[in] other  The replacement argument event.
    ///
    /// @post Strong exception safety guarantees.
    /// @post The complement flag is preserved.
    /// @post The position is preserved.
    ///
    /// @throws DuplicateElementError  The replacement argument is duplicate.
    /// @throws LogicError  The current argument does not belong to this formula.
    /// @throws LogicError  The replacement would result in invalid setup.
    [[maybe_unused]] void Swap(ArgEvent current, ArgEvent other);

  private:
    /// Validates the min and max numbers relevant to the connective.
    ///
    /// @param[in] min_number  The number to be used for connective min number.
    /// @param[in] max_number  The number to be used for connective max number.
    ///
    /// @throws LogicError  The min or max number is invalid or not applicable.
    void ValidateMinMaxNumber(std::optional<int> min_number,
                              std::optional<int> max_number);

    /// Validates the formula connective setup.
    ///
    /// @param[in] min_number  The number to be used for connective min number.
    /// @param[in] max_number  The number to be used for connective max number.
    ///
    /// @throws ValidityError  The connective setup is invalid.
    ///
    /// @pre The connective error info must be tagged outside of this function.
    void ValidateConnective(std::optional<int> min_number,
                            std::optional<int> max_number);

    /// Checks if the formula argument results in invalid nesting.
    ///
    /// @param[in] arg  The argument in the formula.
    ///
    /// @throws LogicError  Invalid nesting of complement or constant args.
    void ValidateNesting(const Arg& arg);

    Connective connective_;  ///< Logical connective.
    std::uint16_t min_number_;  ///< Min number for "atleast"/"cardinality".
    std::uint16_t max_number_;  ///< Max number for "cardinality".
    ArgSet args_;  ///< All events.
};

using FormulaPtr = std::unique_ptr<Formula>;  ///< Convenience alias.

/// Comparison of formula arguments.
inline bool operator==(const Formula::Arg& lhs,
                       const Formula::Arg& rhs) noexcept {
    return lhs.complement == rhs.complement && lhs.event == rhs.event;
}

}