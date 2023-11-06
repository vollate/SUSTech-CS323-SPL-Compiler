#include "Frontage.hpp"
#include <array>
#include <exception>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

using namespace spl;

namespace {}  // namespace

Frontage::Frontage(std::string const& filePath) : m_scanner(*this, filePath), m_parser(m_scanner, *this), m_location() {
    std::vector<std::string> SysIncludePath = { "/usr/lib/gcc/x86_64-pc-linux-gnu/13.2.1/include", "/usr/local/include",
                                                "/usr/lib/gcc/x86_64-pc-linux-gnu/13.2.1/include-fixed", "/usr/include" };
}

bool Frontage::parse() {
    clear();
    m_parser.parse();
    return m_errors.empty();
}

void Frontage::clear() {
    m_location = spl::location();
    m_ast.clear();
    m_includeTree.clear();
    m_errors.clear();
}

template <typename... T>
struct overload : T... {
    using T::operator()...;
};
template <typename... T>
overload(T...) -> overload<T...>;

static void recursiveConvert(std::stringstream& s, const std::unique_ptr<ASTNode>& node, size_t level = 0) {
    bool addLevel = false;
    if(auto type = static_cast<token_type>(node->type); type != token_type::NOTHING) {
        addLevel = true;
        for(int i = 0; i < level; ++i) {
            s << "  ";
        }
        if(type == token_type::NON_TERMINAL) {
            s << std::get<std::string>(node->value) << " (" << node->loc.end.line << ")\n";
        } else if(type == token_type::TYPE || type == token_type::ID) {
            s << (type == token_type::TYPE ? "TYPE: " : "ID: ");
            std::visit(overload{ [&](auto val) { s << val; } }, node->value);
            s << '\n';
        } else if(type == token_type::INT || type == token_type::CHAR || type == token_type::FLOAT) {
            switch(type) {
                case token_type::INT:
                    s << "INT: ";
                    break;
                case token_type::CHAR:
                    s << "CHAR: ";
                    break;
                case token_type::FLOAT:
                    s << "FLOAT: ";
                    break;
                default:
                    break;
            }
            std::visit(overload{ [&](auto& val) { s << val << '\n'; } }, node->value);
        } else {
            std::visit(overload{ [&](auto val) { s << val << '\n'; } }, node->value);
        }
    }
    for(auto& subNode : node->subNodes) {
        recursiveConvert(s, subNode, level + (addLevel ? 1 : 0));
    }
}

std::string Frontage::str() const {
    std::stringstream s;
    for(auto& node : m_ast) {
        recursiveConvert(s, node);
    }
    return s.str();
}

spl::location Frontage::location() const {
    return m_location;
}

void Frontage::increaseLine(int32_t line) {
    m_location.end.line += line;
}

void Frontage::appendError(const string& error) {
    m_errors.push_back(error);
}

bool Frontage::sysFirstInclude(const std::string& name) {
    if(auto path = findHeaderSys(name)) {
        m_includeTree.push_back(std::make_unique<SubFrontage>(name));
        return true;
    }
    return false;
}

bool Frontage::userFirstInclude(const std::string& name) {
    if(auto path = findHeaderUser(name, "")) {
        m_includeTree.push_back(std::make_unique<SubFrontage>(path.value()));
        return true;
    }
    return false;
}

std::string Frontage::error() const {
    std::stringstream s;
    for(auto& error : m_errors) {
        s << error << '\n';
    }
    return s.str();
}

std::optional<std::string> Frontage::findHeaderSys(const std::string& headerName) {
    for(auto& path : Frontage::SysIncludePath) {
        std::ifstream file(path + "/" + headerName);
        if(file.good()) {
            return path + "/" + headerName;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Frontage::findHeaderUser(const std::string& headerName, const std::string filePath) {
    std::ifstream file(filePath + "/" + headerName);
    if(file.good()) {
        return filePath + "/" + headerName;
    }
    return findHeaderSys(headerName);
}
SubFrontage::SubFrontage(const std::string& filePath) : Frontage(filePath) {}
