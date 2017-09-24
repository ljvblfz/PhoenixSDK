{ program 11.1
 extend program 6.1 }
 
program Minmax2(minmax, output);

const n = 20;
var a : array[1..n] of integer;
	i,j : integer;
	minmax : file of integer;
procedure minmax2;
	var i :1..n; u,v,min,max :integer;	
begin min := a[1]; max := min; i := 2;
	while i<n do
	begin u := a[i]; v := a[i+1];
		if u>v then
		begin if u>max then max := u;
			  if v<min then min := v
		end else
		begin if v>max then max := v;
			  if u<min then min := u
		end;
		i := i+2
	end;
	if i=n then
		if a[n]>max then max := a[n]
		else if a[n]<min then min := a[n];
	writeln(min, ' ', max); writeln
end; {minmax2}

begin {read array}
	for i := 1 to n do
		begin read(minmax, a[i]); write(a[i]:3)
		end;
	writeln;
	minmax2;
	for i:= 1 to n do
		begin read(minmax, j); a[i] := a[i]+j; write(a[i]:3)
		end;
	writeln;
	minmax2
end.