{ Test Pascal enumerations. }

program Enum2(output);
type color = (white, red, blue, yellow, purple, green,
		      orange, black);
	 sex = (male, female);
	 day = (mon, tues, wed, thur, fri, sat, sun);	
var
	c : color;
	s : sex;
	d : day;
begin
	c := white;
	s := female;
	d := sat;
	
	c := male;	 { error: bad assignment }
	s := orange; { error: bad assignment }
	d := yellow; { error: bad assignment }
	
	c := s;		 { error: bad assignment }
	s := d;		 { error: bad assignment }
	d := c		 { error: bad assignment }
end.