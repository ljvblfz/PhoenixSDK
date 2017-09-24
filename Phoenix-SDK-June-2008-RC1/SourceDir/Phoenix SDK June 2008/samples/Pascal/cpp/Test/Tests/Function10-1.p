{ Tests function/procedure invocations. 
 This sample performs interop between Pascal and C. 
 In this example, the C file will contain the 
 program entry-point function. }

{module}
function _e(x,y,z: integer): integer;
begin _e := x+y+z 
end;