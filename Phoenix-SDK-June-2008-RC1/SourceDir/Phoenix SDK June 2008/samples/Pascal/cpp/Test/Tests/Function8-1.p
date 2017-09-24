{ Tests function/procedure invocations across Pascal modules. }

program Function8(output);
function e(x,y,z: integer): integer; external;
begin writeln(e(1,2,3))
end.