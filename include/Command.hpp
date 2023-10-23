#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>
#include <string>
#include <vector>

namespace spl {

/**
 * AST node. If you can call it AST at all...
 * It keeps function name and a list of arguments.
 */
class Command {
 public:
  Command(std::string const& name, const std::vector<uint64_t> arguments);
  Command(std::string const& name);
  Command();
  ~Command();

  std::string str() const;
  std::string name() const;

 private:
  std::string m_filePath;
  std::vector<uint64_t> m_args;
};

class InputFile {
 public:
  InputFile(std::string const& filePath);

  std::string str() const;

 private:
  std::string filePath;
};

}  // namespace spl

#endif  // COMMAND_H
