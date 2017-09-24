{ program 4.8.1
 exponentiation with natural exponent using extended ** syntax }

program Exponentiation2(exp, output);

var	y : integer;	x,t,z : real;
	exp : file of real;
begin 
	reset(exp);
	read(exp,x,t); 
	y := round(t);
	z := x**y;
	writeln(x,'\t',z);
end.