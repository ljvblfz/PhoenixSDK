{ Test array packing. }

program Array3(output);

const m = 1; n = 2;
	  u = 4; v = 7;	  
var	ai : array[m..n] of integer;
	ar : array[m..n] of real;
	zi : packed array[u..v] of integer;
	zr : packed array[u..v] of real;
	i,j : integer;
begin
	pack(ai,j,zr);	 {error: mismatched array types}
	unpack(zr,ai,j); {error: mismatched array types}
	
	unpack(ai,j,zr); {error: wrong argument order}
	
	pack(j,j,j);	 {error: wrong argument types}
	unpack(j,j,j);	 {error: wrong argument types}

	pack(ai,j,zi);   {error: mismatched array sizes}
	unpack(zr,ar,j)	 {error: mismatched array sizes}
end.