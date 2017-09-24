{ Tests forward function/procedure declarations. }

program Function6(output);
function f0(x,y,z: integer): integer; forward;
function f0(x,y,z: integer): integer;
{ error: may not repeat parameter lists of forward-declared procedures }
begin f0 := x+y+z;
end;
begin writeln(f0(1,2,3))
end.