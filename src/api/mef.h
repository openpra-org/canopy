#pragma once

#include <iosfwd>
#include <memory>
#include <set>

namespace mef::openpsa {
class model;
}

namespace io::xml {
class document;
}

namespace canopy::api::mef::openpsa {

std::shared_ptr<::mef::openpsa::model> from_xml(const std::istream &istream);

std::shared_ptr<::mef::openpsa::model> from_xml(const std::string &path);
std::shared_ptr<::mef::openpsa::model> from_xml(const ::io::xml::document& doc);

std::set<std::shared_ptr<::mef::openpsa::model>> from_xml(const std::set<std::string> &paths);
std::set<std::shared_ptr<::mef::openpsa::model>> from_xml(const std::set<::io::xml::document> &paths);
} // namespace canopy::mef


namespace canopy::api::mef {

template<typename model_type>
std::shared_ptr<model_type> read_file(const std::string &path);
std::shared_ptr<::mef::openpsa::model> from_xml(const ::io::xml::document& doc);

std::set<std::shared_ptr<::mef::openpsa::model>> from_xml(const std::set<std::string> &paths);
std::set<std::shared_ptr<::mef::openpsa::model>> from_xml(const std::set<::io::xml::document> &paths);
} // namespace canopy::mef

