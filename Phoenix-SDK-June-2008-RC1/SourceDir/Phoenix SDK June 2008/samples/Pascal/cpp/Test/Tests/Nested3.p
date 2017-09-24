{ Test Pascal nested procedures. }
 
program Nested3(output);
const n = 3;
var	x : integer;

	procedure inner;
	begin
		{ error: assigning to non-local constant value }
		n := x
	end;

begin 
	x := 9; inner; writeln(n)
end.