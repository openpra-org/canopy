//
// Created by Arjun Earthperson on 10/17/24.
//

#ifndef CANOPY_LOGICBLOCK_H
#define CANOPY_LOGICBLOCK_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <bitset>

// Templated base class for logic blocks
template<typename Wires>
class LogicBlock {
public:

    LogicBlock(const size_t input_width, const size_t output_width): input_width_(input_width), output_width_(output_width) {}

    virtual ~LogicBlock() = default;

    // Pure virtual method to evaluate the logic block
    virtual void eval(Wires &output, const Wires &input) const = 0;

protected:
    size_t input_width_;
    size_t output_width_;
};

#endif //CANOPY_LOGICBLOCK_H
