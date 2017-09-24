{ Test Pascal nested procedures. }
 
program Nested1(output);
var	x : integer;
	procedure p;	
		begin x := x + 1; writeln(x)
		end;
begin 
	x := 5;	
	writeln(x);
	p;	
	writeln(x)
end.