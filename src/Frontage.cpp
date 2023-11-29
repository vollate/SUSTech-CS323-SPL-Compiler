#include "Frontage.hpp"

#include <fstream>
#include <optional>
#include <sstream>
#include <string>

using namespace spl;

namespace {}  // namespace

Frontage::Frontage(std::string const &filePath) : m_scanner(*this, filePath), m_parser(m_scanner, *this), m_location() {
    std::vector<std::string> SysIncludePath = {"/usr/lib/gcc/x86_64-pc-linux-gnu/13.2.1/include", "/usr/local/include",
                                               "/usr/lib/gcc/x86_64-pc-linux-gnu/13.2.1/include-fixed", "/usr/include"};
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
    m_defTableStack.clear();
    m_varTableStack.clear();
}

template<typename... T>
struct overload : T ... {
    using T::operator()...;
};
template<typename... T>
overload(T...) -> overload<T...>;

static void recursiveConvert(std::stringstream &s, const std::unique_ptr<ParseNode> &node, size_t level = 0) {
    bool addLevel = false;
    if (auto type = static_cast<TOKEN_TYPE>(node->type); type != TOKEN_TYPE::NOTHING) {
        addLevel = true;
        for (int i = 0; i < level; ++i) {
            s << "  ";
        }
        if (type == TOKEN_TYPE::NON_TERMINAL) {
            s << std::get<std::string>(node->typeValue) << " (" << node->loc.end.line << ")\n";
        } else if (type == TOKEN_TYPE::TYPE || type == TOKEN_TYPE::ID) {
            s << (type == TOKEN_TYPE::TYPE ? "TYPE: " : "ID: ");
            std::visit(overload{[&](auto val) { s << val; }}, node->typeValue);
            s << '\n';
        } else if (type == TOKEN_TYPE::INT || type == TOKEN_TYPE::CHAR || type == TOKEN_TYPE::FLOAT) {
            switch (type) {
                case TOKEN_TYPE::INT:
                    s << "INT: ";
                    break;
                case TOKEN_TYPE::CHAR:
                    s << "CHAR: ";
                    break;
                case TOKEN_TYPE::FLOAT:
                    s << "FLOAT: ";
                    break;
                default:
                    break;
            }
            std::visit(overload{[&](auto &val) { s << val << '\n'; }}, node->typeValue);
        } else {
            std::visit(overload{[&](auto val) { s << val << '\n'; }}, node->typeValue);
        }
    }
    for (auto &subNode: node->subNodes) {
        recursiveConvert(s, subNode, level + (addLevel ? 1 : 0));
    }
}

std::string Frontage::str() const {
    std::stringstream s;
    for (auto &node: m_ast) {
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

void Frontage::appendError(const string &error) {
    m_errors.push_back(error);
}

bool Frontage::sysFirstInclude(const std::string &name) {
    if (auto path = findHeaderSys(name)) {
        m_includeTree.push_back(std::make_unique<SubFrontage>(name));
        return true;
    }
    return false;
}

bool Frontage::userFirstInclude(const std::string &name) {
    if (auto path = findHeaderUser(name, "")) {
        m_includeTree.push_back(std::make_unique<SubFrontage>(path.value()));
        return true;
    }
    return false;
}

std::string Frontage::error() const {
    std::stringstream s;
    for (auto &error: m_errors) {
        s << error << '\n';
    }
    return s.str();
}

std::optional<std::string> Frontage::findHeaderSys(const std::string &headerName) {
    for (const auto &path: Frontage::SysIncludePath) {
        std::ifstream file(path + "/" + headerName);
        if (file.good()) {
            return path + "/" + headerName;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Frontage::findHeaderUser(const std::string &headerName, const std::string filePath) {
    std::ifstream file(filePath + "/" + headerName);
    if (file.good()) {
        return filePath + "/" + headerName;
    }
    return findHeaderSys(headerName);
}

SubFrontage::SubFrontage(const std::string &filePath) : Frontage(filePath) {}
