// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "Utility.h"
#include "Exceptions.h"
#include "RuntimeLibrary.h"
#include "FormalParameter.h"
#include "ConstantValue.h"
#include "Output.h"
#include "ValueRange.h"
#include "TypeBuilder.h"
#include "ModuleBuilder.h"
#include "IRBuilder.h"
#include "Configuration.h"

#ifdef _DEBUG
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#endif