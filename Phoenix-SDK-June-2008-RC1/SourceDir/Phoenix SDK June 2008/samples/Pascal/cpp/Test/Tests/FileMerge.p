{ The following program operates on two files of ordered sequences of integers
   f1,f2,...,fm and g1,g2,...,gn
  such that f(i+1) >= f(i) and g(j+1) >= g(j), for all i,j
  and merges them into one ordered file h such that
   h(k+1) >= h(k) for k = 1,2,...,(m+n-1).
}
program FileMerge(output);
var
	endfg : Boolean;
	f,g,h : file of integer;
begin reset(f); reset(g); rewrite(h);
	
	{ Fill first file if empty }
	if eof(f) then 
	begin
	   write(f, 1,2,3,4,4,4,5,9,19,29,100,500);
	   reset(f)
	end;
	
	{ Fill second file if empty }
	if eof(g) then 
	begin
	   write(g, 0,1,2,4,4,4,6,8,18,20,30,250,3000);
	   reset(g)
	end;
	
	endfg := eof(f) or eof(g);
	while not endfg do
	begin if f^<g^ then
		  begin h^ := f^; get(f);
			endfg := eof(f)
		  end else
		  begin h^ := g^; get(g);
			endfg := eof(g)
		  end;
		  put(h)
	end;
	
	while not eof(g) do
	begin h^ := g^; put(h);
		get(g)
	end;
	while not eof(f) do
	begin h^ := f^; put(h);
		get(f)
	end;

	{ Write resuling file }
	writeln(output, 'file h contains:');
	
	reset(h);
	while not eof(h) do
	begin
		Write(output, h^, ' ');
		get(h)
	end;
end.