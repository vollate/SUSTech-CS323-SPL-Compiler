%{
	#include <iostream>
	#include <cstdlib>
	#include "Scanner.hpp"
	#include "Frontage.hpp"
	#include "Parser.hpp"
	#include "location.hh"

	using namespace std;
    using namespace spl;
	#define yyterminate() spl::Parser::make_TERMINATE(spl::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_frontage.increaseLocation(yyleng);

	// !!!WARNING!!!
	// Location API is used, but the location is not initialized, 'cause I'm lazy. When making
	// a token with make_{something} method you can pass detailed token location. Current location
	// is accessible with m_frontage.location() method. All puzzle elements are there - just
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


"int" {return Parser::make_TYPE("This is int",spl::location());}
<<EOF>>     { return yyterminate();/*return Parser::make_TERMINATE(spl::location());*/ }


%%


