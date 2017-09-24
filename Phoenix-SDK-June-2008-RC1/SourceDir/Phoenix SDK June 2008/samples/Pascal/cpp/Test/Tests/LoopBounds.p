{ Test loops on subrange types. }

program LoopBounds(output);

const max = 63;
var i : 1..max; n : integer;
begin 	
	n := 0;
	
	{ Each value within a subrange type can participate in a 
	 for loop. }
	for i := 1 to max do 
		n := n + i;
		
	writeln(n);	
	n := 0;
	
	{ Note how we have to be careful with while loops, though.
     If we wrote this as:
			while i <= max do
			begin 
				n := n + i;
				i := i + 1
			end;
	 then i will exceed the maximum range when i = max. }
	i := 1;
	while i < max do
	begin 
		n := n + i;
		i := i + 1
	end;
	n := n + max;
		
	writeln(n)
end.