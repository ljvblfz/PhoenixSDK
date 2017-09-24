{ Tests Pascal type conversions. }

program ConvertTypes(output);

const rc = 64.456; {static type is real}
var n,nt : integer; r,rt : real;
begin
	n := 56;
	r := 88.88;
	writeln(n);
	writeln(r);
		
	nt := n; rt := r;
	
	n := rt;	{requires run-time type conversion}
	r := nt;	{requires run-time type conversion}
	writeln(n);
	writeln(r);

	n := 5.67;  {conversion will be resolved at compile-time}	
	r := 31;	{conversion will be resolved at compile-time}	
	writeln(n);
	writeln(r);

	n := rc;	{requires run-time type conversion}
	writeln(n)
end.