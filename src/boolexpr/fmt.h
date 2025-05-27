#ifndef BOOLEXPR_FMT_H
#define BOOLEXPR_FMT_H

#include "./operator.h"

namespace std {

template <>
struct formatter<canopy::boolexpr::UnaryOperatorType> {
    enum class Format {
        Regular,
        Pretty,
    };

    Format fmt{Format::Regular};

    constexpr auto parse(const std::format_parse_context &ctx) {
        auto it = ctx.begin();
        const auto end = ctx.end();

        if (it != end && *it != '}') { // If there is a specifier
            if (*it == 'r') {
                fmt = Format::Regular;
            } else if (*it == 'P') {
                fmt = Format::Pretty;
            } else {
                throw std::format_error("invalid presentation for UnaryOperatorType: expected 'r' or 'P'");
            }
            it++;
        }
        // Check if consumed all or next is '}'
        if (it != end && *it != '}') {
            throw std::format_error(
                "invalid format string for UnaryOperatorType: unexpected characters after specifier");
        }
        return it;
    }

    template <typename FormatContext> auto format(const canopy::boolexpr::UnaryOperatorType op, FormatContext &ctx) const {
        std::string_view name = "unknown_unary_op";
        switch (op) {
        case canopy::boolexpr::UnaryOperatorType::NOT:
            name = (fmt == Format::Regular ? "NOT" : "!");
            break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};


template <>
struct formatter<canopy::boolexpr::BinaryOperatorType> {
    enum class Format {
        Regular,
        Pretty,
    };

    Format fmt{Format::Regular};

    constexpr auto parse(const std::format_parse_context& ctx) {
        auto it = ctx.begin();
        const auto end = ctx.end();

        if (it != end && *it != '}') { // If there is a specifier
            if (*it == 'r') {
                fmt = Format::Regular;
            } else if (*it == 'P') {
                fmt = Format::Pretty;
            } else {
                throw std::format_error("invalid presentation for BinaryOperatorType: expected 'r' or 'P'");
            }
            it++;
        }
        // Check if consumed all or next is '}'
        if (it != end && *it != '}') {
            throw std::format_error(
                "invalid format string for Binary: unexpected characters after specifier");
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const canopy::boolexpr::BinaryOperatorType op, FormatContext& ctx) const {
        std::string_view name = "unknown_binary_op";
        switch (op) {
            case canopy::boolexpr::BinaryOperatorType::AND:
                name = (fmt == Format::Regular ? "AND" : "&&");
                break;
            case canopy::boolexpr::BinaryOperatorType::OR:
                name = (fmt == Format::Regular  ? "OR" : "||");
                break;
            case canopy::boolexpr::BinaryOperatorType::XOR: // "XOR" for both as per request
                name = "XOR";
                break;
            case canopy::boolexpr::BinaryOperatorType::IMPLIES:
                name = (fmt == Format::Regular ? "IMPLIES" : "=>");
                break;
        case canopy::boolexpr::BinaryOperatorType::IFF:
                name = (fmt == Format::Regular ? "IFF" : "<=>");
                break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};


template <>
struct std::formatter<canopy::boolexpr::TernaryOperatorType> {
    enum class Format {
        Regular,
        Pretty,
    };

    Format fmt{Format::Regular};


    constexpr auto parse(const std::format_parse_context& ctx) {
        auto it = ctx.begin();
        const auto end = ctx.end();

        if (it != end && *it != '}') { // If there is a specifier
            if (*it == 'r') {
                fmt = Format::Regular;
            } else if (*it == 'P') {
                fmt = Format::Pretty;
            } else {
                throw std::format_error("invalid presentation for BinaryOperatorType: expected 'r' or 'P'");
            }
            it++;
        }
        // Check if consumed all or next is '}'
        if (it != end && *it != '}') {
            throw std::format_error(
                "invalid format string for Binary: unexpected characters after specifier");
        }
        return it;

    }

    template <typename FormatContext>
    auto format(const canopy::boolexpr::TernaryOperatorType op, FormatContext& ctx) const {
        std::string_view name = "unknown_ternary_op";
        switch (op) {
            case canopy::boolexpr::TernaryOperatorType::IF_THEN_ELSE:
                // As per thought process, "IF_THEN_ELSE" for both 'r' and 'P'
                // as "ITE" isn't a standard symbol.
                name = "IF_THEN_ELSE";
                break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};

}


#endif
