{ Tests the Pascal Boolean type. }

program Boolean1(output);

const t = true; f = false;
var b: Boolean;
	bset1, bset2 : set of Boolean;
begin
	
	bset1 := [false, true];
	bset2 := [true];
	
	{ Test logical operators }
	writeln('t and f: ', t and f);
	writeln('t or f: ', t or f);
	writeln('not t: ', not t);
	writeln('not f: ', not f);
	
	writeln;
	
	{ Test relational operators }
	writeln('false = true: ', false = true);
	writeln('false <> true: ', false <> true);
	writeln('false <= true: ', false <= true);
	writeln('false < true: ', false < true);
	writeln('false > true: ', false > true);
	writeln('false >= true: ', false >= true);	
	writeln('false in [false, true]: ', false in bset1);
	writeln('true in [false, true]: ', true in bset1);
	writeln('false in [true]: ', false in bset2);
	writeln('true in [true]: ', true in bset2);
	
	writeln;
	
	{ Test Standard Boolean functions }
	writeln('4 is odd: ', odd(4));
	writeln('5 is odd: ', odd(5));
	writeln('input is eoln: ', eoln(input));
	writeln('output is eoln: ', eoln(output));
	writeln('input is eof: ', eof(input));
	writeln('output is eof: ', eof(output))	
end.