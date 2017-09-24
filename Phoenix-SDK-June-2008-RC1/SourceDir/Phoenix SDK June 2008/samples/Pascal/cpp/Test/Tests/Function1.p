{ Tests function/procedure invocations. }

program Function1(output);
function f: integer;
	function f2: integer;
	begin f2 := 4
	end;
begin f := f2 + f2
end;
begin writeln(f)
end.