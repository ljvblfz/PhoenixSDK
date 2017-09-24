{ Test Pascal pointer types. }

program Pointer1(output);
type
	link = ^person;
	person = record
		next : link; {or 'next : ^person'}
		ss : integer;
		end;	
var
	p,pt : link;
	ssn : array[1..5] of integer;
	i : integer;
begin 
	ssn[1] := 123456789;
	ssn[2] := 987654321;
	ssn[3] := 567891234;
	ssn[4] := 543219876;
	ssn[5] := 135791357;

	p := nil;
	for i := 5 downto 1 do 
	begin
		new(pt); pt^.ss := ssn[i];
		pt^.next := p; p := pt;
	end;
	
	pt := p;
	while pt <> nil do
	begin
		writeln(pt^.ss); pt := pt^.next
	end;
	
	{ free memory }
	pt := p;
	while pt <> nil do
	begin
		p := pt; pt := pt^.next; dispose(p)
	end
end.