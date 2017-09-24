{ Tests function/procedure invocations. }

program Function7(output);
function f0(x,y,z: integer): integer;
begin f0 := x*y*z;
end;
function f0(x,y,z: integer): integer; { error: 'f0' already defined }
begin f0 := x+y+z;
end;
begin writeln(f0(1,2,3))
end.