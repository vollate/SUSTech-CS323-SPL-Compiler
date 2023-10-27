#include "Frontage.hpp"
#include "Command.hpp"

#include <sstream>

using namespace spl;

Frontage::Frontage(const std::string &filePath) :
        m_commands(),
        m_scanner(*this, filePath),
        m_parser(m_scanner, *this),
        m_location(0) {

}

int Frontage::parse() {
    m_location = 0;
    return m_parser.parse();
}

void Frontage::clear() {
        m_commands.clear();
}

std::string Frontage::str() const {
    std::stringstream s;
    s << "Frontage: " << m_commands.size() << " commands received from command line." << endl;
    for (int i = 0; i < m_commands.size(); i++) {
        s << " * " << m_commands[i].str() << endl;
    }
    return s.str();
}

//void Frontage::switchInputStream(std::istream *is) {
//    m_scanner.switch_streams(is, NULL);
//    m_commands.clear();
//}

void Frontage::addCommand(const Command &cmd) {
   m_commands.push_back(cmd);
}

void Frontage::increaseLocation(int32_t loc) {
    m_location += loc;
    cout << "increaseLocation(): " << loc << ", total = " << m_location << endl;
}

int32_t Frontage::location() const {
    return m_location;
}
