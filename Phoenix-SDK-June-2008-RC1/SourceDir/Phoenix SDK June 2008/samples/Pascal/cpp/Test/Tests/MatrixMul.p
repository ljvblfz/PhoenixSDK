{ program 6.3
 matrix multiplication }
 
program MatrixMul(matrixmul, output);

const m = 4; p = 3; n = 2;
var
	i : 1..m; j : 1..n; k : 1..p;
	s : integer;
	a : array[1..m, 1..p] of integer;
	b : array[1..p, 1..n] of integer;
	c : array[1..m, 1..n] of integer;
	matrixmul : file of integer;
begin { assign initial values to a and b }
	for i := 1 to m do
	begin for k := 1 to p do
		  begin read(matrixmul,s); write(s,'\t'); a[i,k] := s
		  end;
		  writeln
	end;
	writeln;
	for k := 1 to p do
	begin for j := 1 to n do
		  begin read(matrixmul,s); write(s,'\t'); b[k,j] := s
		  end;
		writeln
	end;
	writeln;	
	{ multiply a * b }
	for i := 1 to m do
	begin for j := 1 to n do
		  begin s := 0;
			for k := 1 to p do s := s + (a[i,k] * b[k,j]);
			c[i,j] := s; write(s,'\t')
			end;
			writeln
	end;
	writeln
end.