{ Test expression optimization. 
 Try compiling this with '/dumpchktype' versus '/Oe /dumpchktype'.
 Compiling with /Oe should reduce the constant expression to 
 a single value. }

program OptimizeExpression(output);

var n : integer;
begin
	n := (3 + 4) * 2 + 5;
	writeln(n)
end.