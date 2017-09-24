{ program 9.2 -- insert leading blank }

program Insert(insert, output);

var ch : char;
	insert : text;
begin reset(insert);
	while not eof(insert) do
	begin write(' ');
		while not eoln(insert) do
			begin read(insert, ch); write(ch)
			end;
		writeln; read(insert, ch); read(insert, ch);
	end	
end.