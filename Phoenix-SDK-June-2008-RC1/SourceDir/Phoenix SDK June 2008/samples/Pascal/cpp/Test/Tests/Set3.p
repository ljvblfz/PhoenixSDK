{ Test Pascal sets. }

program Set3(output);
type index = 0..63;
var	n : integer;
	intset1, intset2, intset3: set of index;
begin   
	n := 20;		
	intset1 := [3,0,6];
	intset3 := [0..13];
	intset2 := [0..26]-[n];
		
	{set (in)equality}
	
	write('intset3 = intset2?: ');
	if intset3 = intset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('intset3 <> intset2?: ');
	if intset3 <> intset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('[0..20] = [0..20]?: ');
	if [0..20] = [0..20] then
		writeln('yes')
	else
		writeln('no');
	
	write('[0..20] <> [0..20]?: ');
	if [0..20] <> [0..20] then
		writeln('yes')
	else
		writeln('no');
		
	{set inclusion}
	
	write('intset3 subset of intset2?: ');
	if intset3 <= intset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('intset3 superset of intset2?: ');
	if intset3 >= intset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('intset1 subset of intset2?: ');
	if intset1 <= intset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('intset1 superset of intset2?: ');
	if intset1 >= intset2 then
		writeln('yes')
	else
		writeln('no');
		
	writeln;
	
	{set difference}
	write('[0..26]-[',n,']:\r\n\t[');
	for n := 0 to 63 do
		begin
			if n in intset2 then
				write(n,' ')
		end;	
	writeln(']');
	
	{set intersection}
	intset2 := intset1*intset2;
	write('[3,0,6]*([0..26]-[',n,']):\r\n\t[');
	for n := 0 to 63 do
		begin
			if n in intset2 then
				write(n,' ')
		end;
	writeln(']');
	
	{set union}
	intset2 := intset2+[0..13];
	write('([0..26]-[',n,'])+[0..13]:\r\n\t[');
	for n := 0 to 63 do
		begin
			if n in intset2 then
				write(n,' ')
		end;
	writeln(']')
end.
