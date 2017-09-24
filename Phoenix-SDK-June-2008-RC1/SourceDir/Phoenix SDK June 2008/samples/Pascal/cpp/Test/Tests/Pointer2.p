{ Test Pascal pointer types. }

program Pointer2(output);
type color = (white, red, blue, yellow, purple, green,
		      orange, black);
	 car = record
		doors : integer;
		year : integer;
		paint : color;
		end;
var c1,c2 : ^car;
procedure PrintColor(c : color);
begin
	case c of 
		white:	writeln('white');
		red:	writeln('red');
		blue:	writeln('blue');
		yellow:	writeln('yellow');
		purple:	writeln('purple');
		green:	writeln('green');
		orange:	writeln('orange');
		black:	writeln('black');
	end
end;
begin new(c1); new(c2);
	
	c1^.doors := 2;
	c1^.year := 1967;
	c1^.paint := red;
	
	c2^.doors := 4;
	c2^.year := 1988;
	c2^.paint := white;

	write(c1^.doors,'\t',c1^.year,'\t');
	PrintColor(c1^.paint);
	
	write(c2^.doors,'\t',c2^.year,'\t');
	PrintColor(c2^.paint);
	
	{ Deep copy }
	c2^ := c1^;
	c1^.doors := 3;
	
	write(c1^.doors,'\t',c1^.year,'\t');
	PrintColor(c1^.paint);
	
	write(c2^.doors,'\t',c2^.year,'\t');
	PrintColor(c2^.paint);
	
	dispose(c1); dispose(c2)
end.