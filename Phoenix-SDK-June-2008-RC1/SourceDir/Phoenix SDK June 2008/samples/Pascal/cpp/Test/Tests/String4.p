{ Tests the Pascal 'String' type. }

program String4(strings, output);

const n = 30;
type String30 = packed array[1..n] of char;
var s : String30;
	strings : file of String30;
begin
	reset(strings);
	while not eof(strings) do begin
		readln(strings, s); writeln(s)
	end
end.