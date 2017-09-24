//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Declarations for interfacing the flex scanner with its calling context.
//
//-----------------------------------------------------------------------------


#pragma once

extern void    InitScanner(int argc, char* argv[]);
extern "C" int yylex(void);

extern int   LineNumber; // current line in the scanner
extern char* yytext;  // current token in the scanner

// The scanner maps external type names to this integral type.

public enum class SymbolType
{
   Int, String, Boolean, Error, NumSymbolTypes
};

