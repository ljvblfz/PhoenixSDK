{ Tests forward function/procedure declarations. }

program Function4(output);
function f0: integer; forward;
function f0: integer; extern;  { error: 'f0' already declared }
function f0;
begin f0 := 5;
end;
begin writeln(f0)
end.