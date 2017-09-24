{ Test Pascal subrange types. }

program Subrange4(subrange4,output);
type
	index = 0..63; {subrange of integer}	
var
	i : index;
	subrange4 : file of index;
begin
	reset(subrange4);
	while not eof(subrange4) do
	begin
		read(subrange4, i);
		writeln(i)
	end
end.