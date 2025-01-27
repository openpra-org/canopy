/*
 * Copyright (C) 2023 OpenPRA Initiative
 * Copyright (C) 2025 Arjun Earthperson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file
/// Links


#include "core/queue/layer_manager.h"

#include "scram/direct_eval.h"
#include "scram/logger.h"
#include "scram/probability_analysis.h"

namespace scram::core {

    ProbabilityAnalyzer<DirectEval>::ProbabilityAnalyzer(FaultTreeAnalyzer<DirectEval> *fta, mef::MissionTime *mission_time)
        : ProbabilityAnalyzerBase(fta, mission_time) {
        LOG(DEBUG2) << "Re-using PDAG from FaultTreeAnalyzer for ProbabilityAnalyzer";
    }

    inline ProbabilityAnalyzer<DirectEval>::~ProbabilityAnalyzer() noexcept = default;

    double ProbabilityAnalyzer<DirectEval>::CalculateTotalProbability(const Pdag::IndexMap<double> &p_vars) noexcept {
        CLOCK(calc_time);// BDD based calculation time.
        LOG(DEBUG4) << "Calculating probability using monte carlo sampling...";
        const auto num_samples_per_event = this->settings().num_trials();
        const auto batch_size = this->settings().batch_size();
        const auto sample_size = this->settings().sample_size();
        auto pdag = this->graph();
        canopy::queue::layer_manager<std::double_t, std::uint64_t, std::uint32_t> manager(pdag, batch_size, sample_size);
        const auto tally = manager.tally(pdag->root()->index(), num_samples_per_event);
        LOG(DEBUG4) << "Calculated probability " << tally.mean << " in " << DUR(calc_time);
        return tally.mean;
    }
}// namespace scram::core