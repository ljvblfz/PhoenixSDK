{ program 4.5
 compute the cosine using the expansion:
    cos(x) = 1 - x**2/(2*1) + x**4/(4*3*2*1) - ... }

program Cosine(cosine, output);

const	eps = 1e-14; n = 5;
var		x,sx,s,t : real;
		i,k : integer;
		cosine : file of real;
begin	
	for i := 1 to n do
	begin read(cosine, x); t := 1; k := 0; s := 1; sx := sqr(x);
		while abs(t) > eps*abs(s) do
		begin 			
			k := k + 2; t := -t*sx/(k*(k-1));
			s:= s+t
		end;
		writeln(x,'\t',s,'\t',k div 2)
	end
end.