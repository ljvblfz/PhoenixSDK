{ program 9.1 -- frequency count of letters in input file }

program Fcount(charfile);

var ch : char;
	count : array['a'..'z'] of integer;
	letter : set of 'a'..'z';
	charfile : text;
begin letter := ['a'..'z'];
	for ch := 'a' to 'z' do count[ch] := 0;
	while not eof(charfile) do
	begin
		while not eoln(charfile) do
		begin read(charfile, ch);
			if ch in letter then count[ch] := count[ch] + 1;
		end;
		read(charfile, ch)
	end;
	
	for ch := 'a' to 'z' do 
	begin
		if count[ch] > 0 then
			writeln(output, ch, ': ', count[ch]);
	end
end.