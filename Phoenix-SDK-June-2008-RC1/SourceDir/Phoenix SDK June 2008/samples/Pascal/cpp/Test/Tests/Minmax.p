{ program 6.1 
 find the largest and smallest number in a given list }
 
program Minmax(minmax, output);

const n = 20;
var i,u,v,min,max : integer;
	a : array[1..n] of integer;
	minmax : file of integer;
begin

	{ Read array from file }	
	for i := 1 to 20 do
		read(minmax, a[i]);
				
	min := a[1]; max := min; i := 2;
	while i < n do
	begin u := a[i]; v := a[i+1];
		if u > v then
		begin if u > max then max := u;
			  if v < min then min := v
		end else
		begin if v > max then max := v;
			  if u < min then min := u
		end;
		i := i + 2
	end;
	if i = n then
		if a[n] > max then max := a[n]
		else if a[n] < min then min := a[n];
	
	writeln(max,' ', min)
end.