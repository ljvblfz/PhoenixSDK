{ Test the Pascal Text type. }

program TextFile(output);
var
	t : text; { storage for 't' will be transient because 't' }
	c : char; {  does not appear in the program identifier list. }
begin rewrite(t);
	write(t, 'a','b','c');
	reset(t);
	while not eof(t) do 
	begin
		read(t, c); write(c)
	end
end.