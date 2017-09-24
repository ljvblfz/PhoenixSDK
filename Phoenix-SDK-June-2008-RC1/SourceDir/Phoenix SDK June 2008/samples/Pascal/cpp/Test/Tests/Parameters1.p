{ Test Pascal parameter passing. }
 
program Parameters1(output);
var	a,b : integer;
function add(x,y : integer) : integer;
begin add := x+y
end;
begin a := 2; b := 3;	
	c := add(1,1); {error: c not declared}
	add := 2; {error: cannot assign to function}
	a := add; {error: not enough parameters}
	a := add(1); {error: not enough parameters}
	a := add(1,2,3); {error: too many parameters}
	a := add(1,2); {OK}	
	writeln(a,'+(',a,'+',b,')=',add(a,add(a,b))) {OK}
end.