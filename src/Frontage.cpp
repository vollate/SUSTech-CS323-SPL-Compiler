#include "Frontage.hpp"
#include <sstream>

using namespace spl;

Frontage::Frontage(std::string const &filePath)
        : m_scanner(*this, filePath), m_parser(m_scanner, *this), m_location(0) {}

bool Frontage::parse() {
    m_location = 0;
    return m_parser.parse() == 0;
}

void Frontage::clear() {
    m_location = 0;
    m_ast.clear();
}

std::string Frontage::str() const {
    std::stringstream s;
    //TODO
    for (auto &node: m_ast) {
        s << std::get<std::string>(node.value) ;
    }
    //    s << "Frontage: " << m_commands.size() << " commands received from
    //    command line." << endl; for (int i = 0; i < m_commands.size(); i++) {
    //        s << " * " << m_commands[i].str() << endl;
    //    }
    return s.str();
}

void Frontage::append(ASTNode &&node) { m_ast.push_back(std::move(node)); }

void Frontage::increaseLocation(int32_t loc) {
    m_location += loc;
    cout << "increaseLocation(): " << loc << ", total = " << m_location << endl;
}

int32_t Frontage::location() const { return m_location; }
