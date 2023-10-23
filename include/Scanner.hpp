#ifndef SCANNER_H
#define SCANNER_H
#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer spl_FlexLexer // the trick with prefix; no namespace here :(
#include <FlexLexer.h>
#endif

// Scanner method signature is defined by this macro. Original yylex() returns int.
// Sinice Bison 3 uses symbol_type, we must change returned type. We also rename it
// to something sane, since you cannot overload return type.
#undef YY_DECL
#define YY_DECL spl::Parser::symbol_type spl::Scanner::get_next_token()

#include "Parser.hpp" // this is needed for symbol_type

namespace spl {

// Forward declare interpreter to avoid include. Header is added inimplementation file.
    class Frontage;

    class Scanner : public yyFlexLexer {
    public:
        Scanner(Frontage &driver) : m_driver(driver) {}
        virtual ~Scanner() {}
        virtual spl::Parser::symbol_type get_next_token();

    private:
        Frontage &m_driver;
    };

}
#endif
