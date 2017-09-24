{ Tests function/procedure invocations. 
 This sample performs interop between Pascal and C. 
 In this example, the Pascal file will contain the 
 program entry-point function. }

program Function9(output);
function _e(x,y,z: integer): integer; external;
begin writeln(_e(1,2,3))
end.