{ Test Pascal pointer types. }

program Pointer4(output);
type t = record
			a,b,c,d : integer;
		 end;
var x,y,z : ^t;
	i,j,k : ^integer;
begin 
	new(x); new(y); new(z);
	new(i); new(j); new(k);
	
	{ If compiled for native execution, the Pascal runtime should
	 report a memory leak.
	}
end.