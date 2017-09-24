{ Test multidimensional arrays. }

program Array1(output);

const a = 3; b = 5;
	  c = 7; d = 9;	  
var	M : array[a..b] of array[c..d] of integer;
	N : array[a..b,c..d] of integer;
	i,j : integer;
begin 
	i := 4; j := 8;
	M[i][j] := 22;
	N[i,j] := M[i][j];

	writeln(M[i][j]);
	writeln(N[i,j])
end.