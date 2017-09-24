{ program 4.3
 compute h(n) = 1 + 1/2 + 1/3 + ... + 1/n }

program EgRepeat(output);

var n : integer; h : real;
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
	repeat h := h + 1/n; n := n - 1;
	until n=0;
	writeln(h)
end.