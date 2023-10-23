%{
	#include <iostream>
	#include <cstdlib>
	#include "Scanner.hpp"
	#include "Frontage.hpp"
	#include "Parser.hpp"
	#include "location.hh"

	using namespace std;

	#define yyterminate() spl::Parser::make_END(spl::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_driver.increaseLocation(yyleng);

	// !!!WARNING!!!
	// Location API is used, but the location is not initialized, 'cause I'm lazy. When making
	// a token with make_{something} method you can pass detailed token location. Current location
	// is accessible with m_driver.location() method. All puzzle elements are there - just
	// pass location value in every action code block below. I'm going to waste more time writing
	// this excuse than putting this boilerplate below...
	//
	// Location class can be found in location.hh and posistion.hh files. It's just a bit too much
	// boilerplate for this small example. Bummer.
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="spl_"

%%


[a-z]+      {
                cout << "Scanner: identifier [" << yytext << "]" << endl;
                return spl::Parser::make_STRING(yytext, spl::location( /* put location data here if you want */ ));
            }

\(          {
                cout << "Scanner: '('" << endl;
                return spl::Parser::make_LEFTPAR(spl::location());
            }

\)          {
                cout << "Scanner: ')'" << endl;
                return spl::Parser::make_RIGHTPAR(spl::location());
            }

;           {
                cout << "Scanner: ';'" << endl;
                return spl::Parser::make_SEMICOLON(spl::location());
            }

,           {
                cout << "Scanner: ','" << endl;
                return spl::Parser::make_COMMA(spl::location());
            }

[\n\t ]     {
                //cout << "Scanner: whitechar (ignored)" << endl;
            }

[1-9][0-9]* {
                cout << "Scanner: decimal number: " << yytext << endl;
                uint64_t number = strtoull(yytext, 0, 10);
                return spl::Parser::make_NUMBER(number, spl::location());
            }

.           {
                cout << "Scanner: unknown character [" << yytext << "]" << endl;
            }

<<EOF>>     { return yyterminate(); }


%%


