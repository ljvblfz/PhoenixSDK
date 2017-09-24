{ program 4.9
 graphic representation of a function
 f(x) = exp(-x) * sin(2*pi*x) }

program Graph1(output);

const d = 0.0625; {1/16, 16 lines for interval [x,x+1]}
	  s = 32; {32 character widths for interval [y,y+1]}
	  h = 34; {character position of x-axis}
	  c = 6.28318; {2*pi} lim = 32;
var	x,y : real; i,n : integer;
begin 
	for i := 0 to lim do
		begin x := d*i; y := exp(-x)*sin(c*x);
		n := round(s*y) + h;
		repeat write(' '); n := n-1;
		until n=0;
		writeln('*');
	end
end.