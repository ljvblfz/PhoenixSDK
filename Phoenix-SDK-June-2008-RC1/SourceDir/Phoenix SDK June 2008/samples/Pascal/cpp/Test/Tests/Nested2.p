{ Test Pascal nested procedures. }
 
program Nested2(output);
var	x0, y0 : integer;

	procedure outer;
	var	x1, y1 : integer;
		procedure inner;
		var	x2, y2 : integer;
		begin 
			x2 := 3; y2 := 3;
			writeln(x1,' ',y1);
			x1 := x1 + x2;
			y1 := y1 + y2;
			writeln(x1,' ',y1)			
		end;
	begin
		x1 := 13; y1 := 13;
		inner;
		x0 := x0 + x1;
		y0 := y0 + y1	
	end;

begin 
	x0 := 33; y0 := 32;
	
	writeln(x0,' ',y0);
	outer;	
	writeln(x0,' ',y0)
end.