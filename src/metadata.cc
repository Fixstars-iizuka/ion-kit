#include <sstream>

#include "ion/generator.h"
#include "ion/json.hpp"
#include "ion/port.h"

#include "metadata.h"

namespace {

std::string unquote(const std::string& s) {
    if (s.size() < 2) {
        return s;
    }

    size_t epos = s.size()-1;

    if (s[0] == '"' && s[epos] == '"') {
        return s.substr(1, s.size()-2);
    } else {
        return s;
    }
}

}

namespace ion {

using json = nlohmann::json;

PortMD::PortMD(const std::string& n, Halide::Type t, int d)
    : name(n), type(t), dimension(d)
{}

void to_json(json& j, const PortMD& v) {
    j["name"] = v.name;
    j["type"] = static_cast<halide_type_t>(v.type);
    j["dimension"] = v.dimension;
}

void from_json(const json& j, PortMD& v) {
    v.name = j["name"].get<std::string>();
    v.type = j["type"].get<halide_type_t>();
    v.dimension = j["dimension"];
}

ParamMD::ParamMD(const std::string& n, const std::string& dv)
    : name(n), default_value(dv)
{}

void to_json(json& j, const ParamMD& v) {
    j["name"] = v.name;
    j["default_value"] = v.default_value;
}

void from_json(const json& j, ParamMD& v) {
    v.name = j["name"].get<std::string>();
    v.default_value = j["default_value"].get<std::string>();
}

Metadata::Metadata(const std::string& n)
    : name(n)
{
    auto bb = Internal::GeneratorRegistry::create(n, GeneratorContext(Halide::get_host_target()));

    // NOTE: Call fake_configure just to get default value of Param
    bb->fake_configure();

    for (auto info : bb->param_info().inputs()) {
        auto type = info->types_defined() ? info->type() : Halide::Type();
        auto dims = info->dims_defined() ? info->dims() : -1;
        inputs.push_back(PortMD(info->name(), type, dims));
    }
    for (auto info : bb->param_info().outputs()) {
        auto type = info->types_defined() ? info->type() : Halide::Type();
        auto dims = info->dims_defined() ? info->dims() : -1;
        outputs.push_back(PortMD(info->name(), type, dims));
    }
    for (auto info : bb->param_info().generator_params()) {
        auto dv = info->is_synthetic_param() ? "" : unquote(info->get_default_value());
        params.push_back(ParamMD(info->name, dv));
    }
}

void to_json(json& j, const Metadata& v) {
    j["name"] = v.name;
    j["inputs"] = v.inputs;
    j["outputs"] = v.outputs;
    j["params"] = v.params;
}

void from_json(const json& j, Metadata& v) {
    v.name = j["name"].get<std::string>();
    v.inputs = j["inputs"].get<std::vector<PortMD>>();
    v.outputs = j["outputs"].get<std::vector<PortMD>>();
    v.params = j["params"].get<std::vector<ParamMD>>();
}

} //namespace ion
