#ifndef BOOLEXPR_UTIL_NAME_INDEXER_H
#define BOOLEXPR_UTIL_NAME_INDEXER_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>

#include "boolexpr/util/helper_concepts.h"

namespace canopy::boolexpr::util {

template <UnsignedInteger ID> class NameIndexer {
  public:

    std::optional<ID> find(std::string_view name) const {
        if (auto it = name_to_id_.find(name); it != name_to_id_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    ID find_or_insert(const std::string_view name) {
        if (const auto id = find(name); id.has_value()) {
            return id.value();
        }

        return insert(name);
    }

    std::optional<std::string_view> get(ID id) const {
        if (id >= name_to_id_.size()) {
            return std::nullopt;
        }

        return id_to_name_.at(id);
    }

  private:
    ID insert(std::string_view name) {
        const auto new_id = id_to_name_.size();
        id_to_name_.emplace_back(name);
        name_to_id_[name] = new_id;
        return new_id;
    }

    std::vector<std::string> id_to_name_{};
    std::unordered_map<std::string_view, ID> name_to_id_{};
};

} // namespace canopy::boolexpr::util

#endif
