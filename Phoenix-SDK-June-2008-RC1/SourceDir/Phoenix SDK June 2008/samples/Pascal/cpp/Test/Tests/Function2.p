{ Tests forward function/procedure declarations. }

program Function2(output);
function f0: integer; forward;
function f1(n: integer): integer; forward;
function f2: integer;
	begin f2 := f0 + f1(7)
	end;
function f0;
{ Return value and argument list are omitted because
 thre was a forward declaration above }
begin f0 := 5;
end;
function f1;
{ Return value and argument list are omitted because
 thre was a forward declaration above }
begin f1 := 2*n
end;
begin writeln(f2)
end.