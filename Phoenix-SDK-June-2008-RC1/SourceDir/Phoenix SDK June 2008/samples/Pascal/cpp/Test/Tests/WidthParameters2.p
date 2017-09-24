{ Test Pascal write/writeln width parameters. }
 
program WidthParameters2(output);

begin

	{ Char }
	writeln('a':1:1);     {error: invalid width specifier}
	{ Integer }
	writeln(44:1:1);      {error: invalid width specifier}
	{ Boolean }
	writeln(true:4:4);    {error: invalid width specifier}
	{ String }
	writeln('hello':4:4); {error: invalid width specifier}
	{ Real }
	writeln(2.2:2:2)	  {ok: valid width specifier}
	
end.