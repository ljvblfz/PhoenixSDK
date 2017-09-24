{ Test Pascal write/writeln width parameters. }
 
program WidthParameters1(output);

const pi = 3.14159265358979323846;
var i : integer;
begin

	{ Char }
	writeln('a':0);
	writeln('a':1);
	writeln('a':2);
	writeln('a':3);
	writeln('a':4);
	
	writeln;
	
	{ Integer }
	i := 44;
	writeln(i:0);
	writeln(i:1);
	writeln(i:2);
	writeln(i:3);
	writeln(i:4);
	
	writeln;
	
	{ Boolean }
	writeln(true:0);
	writeln(true:4);
	writeln(true:8);
	
	writeln(false:0);
	writeln(false:4);
	writeln(false:8);
	
	writeln;
	
	{ String }
	writeln('hello':0);
	writeln('hello':4);
	writeln('hello':8);
	
	writeln;
	
	{ Real }
	writeln(pi:0);
	writeln(pi:0:2);
	writeln(pi:2);
	writeln(pi:2:2);
	writeln(pi:4);
	writeln(pi:4:4);
	writeln(pi:8);
	writeln(pi:8:8)
	
end.