{ program 4.4
 compute h(n) = 1 + 1/2 + 1/3 + ... + 1/n }

program EgFor(output);

var i, n : integer; h : real;
	eg : file of integer;
begin 
	if eof(eg) then
	begin
		rewrite(eg); write(eg, 10)
	end;
	
	reset(eg);	
	read(eg, n);
	write(n, ' ');
	h := 0;
	for i := n downto 1 do h := h + 1/i;
	writeln(h)
end.