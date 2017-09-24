{ program 11.2
 extend program 11.1 }
 
program Minmax3(minmax3, output);

const n = 20;
type list = array[1..n] of integer;
var a,b : list;
	i,min1,min2,max1,max2 : integer;
	minmax3 : file of integer;
	
procedure minmax(var g:list; var j,k:integer);
	var i :1..n; u,v :integer;	
begin j := g[1]; k := j; i := 2;	
	while i<n do
	begin u := g[i]; v := g[i+1];
		if u>v then
		begin if u>k then k := u;
			  if v<j then j := v
		end else
		begin if v>k then k := v;
			  if u<j then j := u
		end;
		i := i+2
	end;
	if i=n then
		if g[n]>k then k := g[n]
		else if g[n]<j then j := g[n];
end; {minmax}

begin {read array}
	for i := 1 to n do
		begin read(minmax3, a[i]); write(a[i]:3) end;
	writeln;
	minmax(a,min1,max1);
	writeln(min1:3,max1:3,max1-min1:3); writeln;	
	for i:= 1 to n do
		begin read(minmax3, b[i]); write(b[i]:3) end;	
	writeln;
	minmax(b,min2,max2);
	writeln(min2:3,max2:3,max2-min2:3);
	writeln(abs(min1-min2):3,abs(max1-max2):3); writeln;
	for i := 1 to n do
		begin a[i] := a[i] + b[i]; write(a[i]:3) end;
	writeln;
	minmax(a,min1,max1);
	writeln(min1:3,max1:3,max1-min1:3)
end.