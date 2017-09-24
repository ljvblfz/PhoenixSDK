{ Tests forward function/procedure declarations. }

program Function3(output);
function f0: integer; static;	{ error: 'static' is not a valid directive }
function f1(n: integer): integer; static;	{ error: 'static' is not a valid directive }
function f2: integer;
	begin f2 := f0 + f1
	end;
function f0;
begin f0 := 5;
end;
function f1;
begin f1 := 2*n;
end;
begin writeln(f2)
end.