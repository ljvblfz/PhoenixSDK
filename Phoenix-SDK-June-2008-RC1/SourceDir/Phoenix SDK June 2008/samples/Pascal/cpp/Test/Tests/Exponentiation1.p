{ program 4.8
 exponentiation with natural exponent }

program Exponentiation1(exp, output);

var	e,y : integer;	u,x,t,z : real;
	exp : file of real;
begin 
	reset(exp);
	read(exp,x,t); 
	y := round(t);
	write(x,'\t',y,' ');
	z := 1; u := x; e := y;
	while e > 0 do
	begin { z*u**e = x**y, e>0 }
		while not odd(e) do
			begin e := e div 2; u := sqr(u)
			end;
		e := e - 1; z := u*z;
	end;
	writeln(z) { z = x**y }
end.