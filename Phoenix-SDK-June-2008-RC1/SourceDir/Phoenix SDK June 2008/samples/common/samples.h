//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Common macros and typedefs for code samples
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit each may partial overlapping alias of an alias tag.
//
// Arguments:
//
//    aliasTag - Iterated tag member.
//    tag - Tag whose total aliases are to enumerate.
//    info - Alias info class.
//
//-----------------------------------------------------------------------------

#define foreach_may_partial_alias_of_tag(aliasTag, tag, info)                  \
   {                                                                           \
      /* dummy variable to pair foreach/next */                                \
                                                                               \
      Alias::MemberPosition _memPos;                                           \
      Phx::Boolean    _isIterMayPartialAliasOfTag = false;                     \
      Phx::Alias::Tag aliasTag = info->GetFirstMayPartialAlias(tag, &_memPos); \
                                                                               \
      for (;                                                                   \
           aliasTag != Phx::Alias::Constants::InvalidTag;                      \
           aliasTag = info->GetNextMayPartialAlias(tag, &_memPos))             \
      {

//-----------------------------------------------------------------------------
//
// Description:
//
//    next for foreach_may_partial_alias_of_tag
//
//-----------------------------------------------------------------------------

#define next_may_partial_alias_of_tag                   \
      }                                                 \
                                                        \
      /* dummy variable to matchup foreach/next */      \
                                                        \
      _isIterMayPartialAliasOfTag = false;              \
   }

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit each must total overlapping alias of an alias tag.
//
// Arguments:
//
//    aliasTag - Iterated tag member.
//    tag - Tag whose total aliases are to enumerate.
//    info - Alias info class.
//
//-----------------------------------------------------------------------------

#define foreach_must_total_alias_of_tag(aliasTag, tag, info)                   \
   {                                                                           \
      /* dummy variable to pair foreach/next */                                \
                                                                               \
      Alias::MemberPosition _memPos;                                           \
      Phx::Boolean    _isIterMustTotalAliasOfTag = false;                      \
      Phx::Alias::Tag aliasTag = info->GetFirstMustTotalAlias(tag, &_memPos);  \
                                                                               \
      for (;                                                                   \
           aliasTag != Phx::Alias::Constants::InvalidTag;                      \
           aliasTag = info->GetNextMustTotalAlias(tag, &_memPos))              \
      {

//-----------------------------------------------------------------------------
//
// Description:
//
//    next for foreach_must_total_alias_of_tag
//
//-----------------------------------------------------------------------------

#define next_must_total_alias_of_tag                    \
      }                                                 \
                                                        \
      /* dummy variable to matchup foreach/next */      \
                                                        \
      _isIterMustTotalAliasOfTag = false;               \
   }

namespace Phx 
{

typedef __int32          Int32;
typedef bool             Boolean;
typedef wchar_t          Character;
typedef int              Int; // Generic signed integer
typedef unsigned int     UInt; // Generic unsigned integer
typedef unsigned __int8  UInt8;
typedef unsigned __int32 UInt32;

typedef UInt32           Id; // Identifier
typedef UInt32           ExternId; // External identifier

typedef UInt32           ByteSize; // An size in bytes
typedef UInt32           BitNumber;  // unsigned for ease of bit vector use
typedef Int32            BitOffset; // An offset in bits
typedef Int32            ByteOffset; // An offset in bytes

typedef Int32            RegNum;   // needs to be int for extensibility
typedef Int32            RegClassNum;   // needs to be int for extensibility

namespace Alias
{

typedef Phx::Int32 Tag;                 // Currently must be signed.

} // namespace Alias
}
