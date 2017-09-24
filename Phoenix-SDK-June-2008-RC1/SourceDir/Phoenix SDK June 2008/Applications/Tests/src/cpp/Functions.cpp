//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Funcs.cpp : a few functions, used to demonstrate operation of the
//    "FuncNames" sample plug-in.
//
// Usage:
//
//    See FuncNames.cs for full information on how the FuncNames plug-in
//    is run with C2 as host.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    A dummy function called "f1" that does nothing.  As with "f2" and "f3",
//    we declare the function extern "C" only so that, when the FuncNames
//    plug-in 'sees' this function, it reports its name as "_f1" rather than
//    its mangled C++ name.
//
//-----------------------------------------------------------------------------

extern "C" void
f1(
)
{
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    A dummy function called "f2" that does nothing.
//
//-----------------------------------------------------------------------------

extern "C" void
f2
(
)
{
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    A dummy function called "f3" that does nothing.
//
//-----------------------------------------------------------------------------

extern "C" void
f3
(
)
{
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    The main method.
//
// Returns:
//
//    No explicit return value, so C++ manufactures return value of 0.
//
//-----------------------------------------------------------------------------

extern "C" int
main
(
)
{
   ::f1();
   ::f2();
   ::f3();
}