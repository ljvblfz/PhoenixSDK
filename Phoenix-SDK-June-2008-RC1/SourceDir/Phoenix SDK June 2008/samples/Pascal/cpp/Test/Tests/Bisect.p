{ program 11.6
 find zero of a function by bisection }
 
program Bisect(bisect, output);

const eps = 1e-14;
var x,y : real; bisect : file of real;

function zero(function f(q:real): real; a,b: real): real;
var x,z : real; s: Boolean;
begin s := f(a)<0;
	repeat x := (a+b)/2.0;		
		z := f(x);
		if (z<0)=s then a := x else b := x
	until abs(a-b)<=eps;
	zero := x
end; {zero}

begin {main}
	read(bisect,x,y); writeln(x,' ',y,' ',zero(sin,x,y));
	read(bisect,x,y); writeln(x,' ',y,' ',zero(cos,x,y))
end.