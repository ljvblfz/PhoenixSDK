//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of custom phase classes.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;

//-----------------------------------------------------------------------------
//
// Description: Optimization phase.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class OptimizationPhase : public Phx::Phases::Phase
{
public:

   static OptimizationPhase ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

protected:

   virtual void
   Execute
   (
     Phx::Unit ^ unit
   ) override;
};

//-----------------------------------------------------------------------------
//
// Description: Encoding phase.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class Encode : public Phx::Phases::Phase
{
public:

   static Encode ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

protected:

   virtual void
   Execute
   (
     Phx::Unit ^ unit
   ) override;
};

//-----------------------------------------------------------------------------
//
// Description: Phase for producing an assembly listing.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class Lister : Phx::Phases::Phase
{
public:

   static Lister ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

protected:

   virtual void
   Execute
   (
      Phx::Unit ^ unit
   ) override;
};