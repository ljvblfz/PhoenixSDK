{ Test Pascal enumerations. }

program Enum1(output);
type color = (white, red, blue, yellow, purple, green,
		      orange, black);
	 sex = (male, female);
	 day = (mon, tues, wed, thur, fri, sat, sun);	
var
	c : color;
	weekend : sat..sun;

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

begin

	for c := black downto red do
		PrintColor(c);
		
	writeln;
	
	c := pred(c);
	while c <> black do
	begin
		PrintColor(c);
		c := succ(c);
	end;
	
end.