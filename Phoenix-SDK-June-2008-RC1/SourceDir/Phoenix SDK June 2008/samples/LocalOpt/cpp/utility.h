//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    A thin wrapper of System.Collections.Generics.Dictionary<TKey, TValue>.
//    In the system generic class, if a key to insert is already in the 
//    Dictionary, an ArgumentException will be thrown. In our case, we often 
//    want to replace the value of certain key in the Dictionary. So we need to
//    either use the try/catch block or check before insert to avoid unhandled
//    exception every time we need to insert or update an entry in the table.
//    To simply our code, the following subclass factors out the check-then-
//    insert code sequence and provide two frequently used operation: Insert 
//    and Lookup.
//
//-----------------------------------------------------------------------------

#pragma once

namespace Phx
{

namespace Samples
{

namespace LocalOpt
{
        
generic <class TKey, class TValue>	public ref class Hashtable : 
        System::Collections::Generic::Dictionary<TKey,TValue> 
{

public: 

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Constructor of the Hashtable class
   //
   // Arguments:
   //
   //    size - The initial size of the hashtable
   //
   // Returns:
   //
   //    A new instance of Hashtable
   //
   // Remarks:
   //
   //    Simply call contructor of the base class
   //
   //-------------------------------------------------------------------------- 
   
   Hashtable 
   (
      int size
   ) : 
      System::Collections::Generic::Dictionary<TKey, TValue>(size)
   {
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Look up an object in the hash table using the given key
   //
   // Arguments:
   //
   //    key - The key to lookup
   //
   // Returns:
   //
   //    The value if found, otherwise the default value of TValue
   //
   //--------------------------------------------------------------------------


	TValue 
   Lookup 
   (
      TKey key
   )
   {
 	   TValue value;
	
      if (this->TryGetValue(key, value))
	   {
		   return value;
	   } 
      else
	   {
		   return TValue();
	   }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Insert an object in the hash table. If the key was previously 
   //    associated to another value, update the value to the new object
   //
   // Arguments:
   //
   //    key - The key of the pair to be inserted
   //    value - The value of the pair to be inserted
   //
   // Returns:
   //
   //    True if the key was not previosly associated with a value;
   //    false otherwise.
   //
   //--------------------------------------------------------------------------

	bool 
   Insert 
   (
      TKey key, 
      TValue value
   )
   {
      bool isNew = true;
	
      if (this->ContainsKey(key))
	   {
		   this->Remove(key);
		   isNew = false;
	   }
	
      this->Add(key, value);
	   return isNew;
   }

};


} // namespace LocalOpt
} // namespace Samples
} // namespace Phx
