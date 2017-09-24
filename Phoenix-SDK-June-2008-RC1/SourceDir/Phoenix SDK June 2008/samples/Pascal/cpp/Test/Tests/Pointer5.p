{ Test Pascal pointer types. }

program Pointer5(output);
type t = record
			e : integer;
		 end;
var x : ^t;
begin 
	new(x);
	
	x^.e := 5;
	x.e := 3;		{ error: missing pointer access symbol }
	
	writeln(x^.e);
	writeln(x.e);	{ error: missing pointer access symbol }
	
	dispose(x)
end.