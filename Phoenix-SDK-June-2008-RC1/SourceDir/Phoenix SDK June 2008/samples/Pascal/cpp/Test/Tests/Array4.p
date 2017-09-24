{ Test Pascal arrays. }

program Array4(output);
const n = 3;
type complex = record re, im : real	end;
var a : array[1..n] of integer;
	b : array[(A,B,C)] of integer;
	c : array[1..2, (D,E)] of integer;
	d : array[1..2, 1..2, 1..2] of integer;
	e : array[1..2, 100..300, 1..20, 1..5] of char;
	i, j, k, l : integer;
	next : char;
	first : boolean;
	x : array[1..10] of complex;
	re, im : real;
begin		
	d[1,1,1] := 0;	
	d[1,1,2] := 1;
	d[1,2,1] := 2;	
	d[1,2,2] := 3;	
	d[2,1,1] := 4;
	d[2,1,2] := 5;
	d[2,2,1] := 6;
	d[2,2,2] := 7;	
	writeln(d[1,1,1]);
	writeln(d[1,1,2]);
	writeln(d[1,2,1]);
	writeln(d[1,2,2]);	
	writeln(d[2,1,1]);
	writeln(d[2,1,2]);
	writeln(d[2,2,1]);
	writeln(d[2,2,2]);
	writeln;
	
	c[1, D] := 1;
	c[2, D] := 2;
	c[1, E] := 3;
	c[2, E] := 4;	
	writeln(c[1, D]);
	writeln(c[2, D]);
	writeln(c[1, E]);
	writeln(c[2, E]);
	
	b[A] := 0;
	b[B] := 1;
	b[C] := 2;
	writeln(b[A]);
	writeln(b[C]);
	writeln(b[B]);
	
	i := 1;	
	a[i+1] := 35;
	a[2] := -68;
	a[3] := 94;
	a[i] := 22;		
	writeln(a[i+1]);
	writeln(a[2]);
	writeln(a[3]);
	writeln(a[i]);
		
	first := true;
	for i := 1 to 2 do
	begin
		for j := 100 to 300 do
		begin
			for k := 1 to 20 do
			begin
				next := 'A';
				for l := 1 to 5 do
				begin
					e[i,j,k,l] := next; 
					if first then write(e[i,j,k,l]:2);
					next := succ(next)
				end;
				if first then writeln;
				first := false;
			end;
		end;
	end;
		
	re := 0.0;
	im := 1.0;
	for i := 1 to 10 do
	begin
		x[i].re := re; x[i].im := im;
		writeln(x[i].re, '.', x[i].im);
		re := re + 0.5; im := im + re
	end	
end.