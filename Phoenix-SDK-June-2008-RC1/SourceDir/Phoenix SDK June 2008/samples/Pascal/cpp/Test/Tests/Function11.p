{ Tests function/procedure invocations. }

program Function11(output);
	function e(x: real): real;
	var z : integer;
		function f(y: real): real;
		begin
			f := x + y + z
		end;
	begin
		z := 12;
		e := f(3)
	end;
begin writeln(e(6))
end.