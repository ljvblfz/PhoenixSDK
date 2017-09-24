{ Tests forward function/procedure declarations. }

program Function5(output);
function f0: integer; extern;
function f0;
{ error: externally-declared functions must appear
 in an external module }
begin f0 := 5;
end;
begin writeln(f0)
end.