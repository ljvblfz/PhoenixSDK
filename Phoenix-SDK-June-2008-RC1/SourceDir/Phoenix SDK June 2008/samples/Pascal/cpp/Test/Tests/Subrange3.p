{ Test Pascal subrange types. }

program Subrange3(output);
type
	index = 1..63;
var
	i : index;
begin
	for i := 60 to 64 do
		writeln(i,' ');
end.