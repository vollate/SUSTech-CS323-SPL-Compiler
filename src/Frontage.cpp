#include "Frontage.hpp"
#include <sstream>

using namespace spl;

Frontage::Frontage(std::string const& filePath) : m_scanner(*this, filePath), m_parser(m_scanner, *this), m_location(0) {}

bool Frontage::parse() {
    clear();
    return m_parser.parse() == 0;
}

void Frontage::clear() {
    m_location = 0;
    m_ast.clear();
    emptyParentStack();
}
static void recursiveConvert(std::stringstream& s, const std::unique_ptr<ASTNode>& node) {
    // TODO: add type checking @JYF
    try {
        s << " " << std::get<int>(node->value);
    } catch(std::bad_variant_access&) {
        try {
            s << " " << std::get<float>(node->value);
        } catch(std::bad_variant_access&) {
            try {
                s << " " << std::get<std::string>(node->value);
            } catch(std::bad_variant_access&) {
                s << " "
                  << "error";
            }
        }
    }
    for(auto& subNode : node->subNodes) {
        recursiveConvert(s, subNode);
    }
}
std::string Frontage::str() const {  // TODO
    std::stringstream s;
    for(auto& node : m_ast) {
        recursiveConvert(s, node);
    }
    return s.str();
}

void Frontage::increaseLocation(int32_t loc) {  // TODO:修正位置信息@JYF
    m_location += loc;
    cout << "increaseLocation(): " << loc << ", total = " << m_location << endl;
}

int32_t Frontage::location() const {
    return m_location;
}

void Frontage::pushParent(ASTNode* node) {
    if(node) {
        m_parentNodeStack.push(node);
    } else {
        m_parentNodeStack.push(m_ast.back().get());
    }
}

ASTNode* Frontage::getParent() {
    if(!m_parentNodeStack.empty()) {
        return m_parentNodeStack.top();
    } else if(!m_ast.empty()) {
        return m_ast.back().get();
    }
    return nullptr;
}

void Frontage::emptyParentStack() {
    while(!m_ast.empty()) {
        m_ast.pop_back();
    }
}

ASTNode* Frontage::selectParent(const string& nonTerminalName) {
    auto parent = getParent();
    for(auto& candidate : parent->subNodes) {
        if(std::get<std::string>(candidate->value) == nonTerminalName) {
            return candidate.get();
        }
    }
    return nullptr;
}
