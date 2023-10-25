#include "Command.hpp"

#include <iostream>
#include <sstream>

using namespace spl;
using std::cout;
using std::endl;

Command::Command(const std::string &name, const std::vector<uint64_t> arguments) :
        m_filePath(name),
        m_args(arguments) {
}

Command::Command(const std::string &name) :
        m_filePath(name),
        m_args() {
}

Command::Command() :
        m_filePath(),
        m_args() {
}

Command::~Command() {
}

std::string Command::str() const {
    std::stringstream ts;
    ts << "name = [" << m_filePath << "], ";
    ts << "arguments = [";

    for (int i = 0; i < m_args.size(); i++) {
        ts << m_args[i];
        if (i < m_args.size() - 1) {
            ts << ", ";
        }
    }

    ts << "]";
    return ts.str();
}

std::string Command::name() const {
    return m_filePath;
}

InputFile::InputFile(const std::string &filePath) : filePath(filePath) {}