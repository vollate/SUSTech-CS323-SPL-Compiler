#include "Frontage.hpp"
#include <sstream>

using namespace spl;

Frontage::Frontage(std::string const &filePath) : m_scanner(*this, filePath), m_parser(m_scanner, *this),
                                                  m_location(0) {}

bool Frontage::parse() {
    clear();
    return m_parser.parse() == 0;
}

void Frontage::clear() {
    m_location = spl::location();
    m_ast.clear();
}

template<typename ...T>
struct overload : T ... {
    using T::operator()...;
};
template<typename ...T>
overload(T...) -> overload<T...>;

static void recursiveConvert(std::stringstream &s, const std::unique_ptr<ASTNode> &node, size_t level = 0) {
    for (int i = 0; i < level; i++) {
        s << "  ";
    }
    if (static_cast<token_type>(node->type) == token_type::NON_TERMINAL) {
        std::visit(overload{[&](auto val) { s << val; }}, node->value);
        s << '(' << node->loc.begin.line << ')' << '\n';
    } else if (auto type = static_cast<token_type>(node->type);type == token_type::TYPE || type == token_type::ID) {
        s << (type == token_type::TYPE ? "TYPE: " : "ID: ");
        std::visit(overload{[&](auto val) { s << val; }}, node->value);
        s << '\n';
    }else {
        std::visit(overload{[&](auto val) { s << val<<'\n'; }}, node->value);
    }
    for (auto &subNode: node->subNodes) {
        recursiveConvert(s, subNode, level + 1);
    }
}

std::string Frontage::str() const {  // TODO
    std::stringstream s;
    for (auto &node: m_ast) {
        recursiveConvert(s, node);
    }
    return s.str();
}

void Frontage::increaseLocation(int32_t loc) {  // TODO:修正位置信息@JYF
    m_location += loc;
//    cout << "increaseLocation(): " << loc << ", total = " << m_location << endl;
}

spl::location Frontage::location() const {
    return m_location;
}

void Frontage::increaseLine(int32_t line) {
    m_location.end.line += line;
}
