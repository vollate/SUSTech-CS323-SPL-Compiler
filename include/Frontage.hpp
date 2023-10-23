#ifndef Frontage_H
#define Frontage_H

#include <vector>

#include "Scanner.hpp"
#include "Parser.hpp"

namespace spl {

// forward declare our simplistic AST node class so we
// can declare container for it without the header
    class Command;

/**
 * This class is the interface for our scanner/scanner. The end user
 * is expected to use this. It drives scanner/scanner, keeps
 * parsed AST and generally is a good place to store additional
 * context data. Both parser and scanner have access to it via internal
 * references.
 * 
 * I know that the AST is a bit too strong word for a simple
 * vector with nodes, but this is only an example. Get off me.
 */
    class Frontage {
    public:
        Frontage(const std::string&filePath);

        /**
         * Run parser. Results are stored inside.
         * \returns 0 on success, 1 on failure
         */
        int parse();

        /**
         * Clear AST
         */
        void clear();

        /**
         * Print AST
         */
        std::string str() const;

        /**
         * Switch scanner input stream. Default is standard input (std::cin).
         * It will also reset AST.
         */
//        void switchInputStream(std::istream *is);

        /**
         * This is needed so that Scanner and Parser can call some
         * methods that we want to keep hidden from the end user.
         */
        friend class Parser;

        friend class Scanner;

    private:
        Scanner m_scanner;
        Parser m_parser;
        std::vector<Command> m_commands;  // Example AST
        int32_t m_location;          // Used by scanner

        // Used internally by Parser to insert AST nodes.
//        void addCommand(const Command &cmd);

        // Used internally by Scanner YY_USER_ACTION to update location indicator
        void increaseLocation(int32_t loc);

        // Used to get last Scanner location. Used in error messages.
        int32_t location() const;


    };

}

#endif // Frontage_H
