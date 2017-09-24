{ Test array packing. }

program Array2(output);

const m = 3; n = 8;
	  u = 4; v = 7;	  
var	a : array[m..n] of integer;
	z : packed array[u..v] of integer;
	i,j : integer;
begin
	write('a =');
	for i := m to n do begin a[i] := i; write(i:2) end;
	writeln;
	write('z =');
	for i := u to v do begin z[i] := i; write(i:2) end;
	writeln;
			
	for j := 0 to 1 do 
	begin
		pack(a,j,z);
		writeln('pack(a,',j,',z)');
		
		write('a =');
		for i := m to n do write(a[i]:2);
		writeln;
		write('z =');
		for i := u to v do write(z[i]:2);
		writeln
	end;
	
	for j := 0 to 1 do 
	begin
		unpack(z,a,j);	
		writeln('unpack(z,a,',j,')');
		
		write('a =');
		for i := m to n do write(a[i]:2);
		writeln;
		write('z =');
		for i := u to v do write(z[i]:2);
		writeln
	end
end.