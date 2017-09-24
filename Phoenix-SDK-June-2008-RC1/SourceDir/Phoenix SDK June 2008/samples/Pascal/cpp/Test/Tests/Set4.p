{ Test Pascal sets. }

program Set4(output);

type fruit = (orange,apple,banana);
	 color = (red,blue,green);
var	c : set of color;
	f : set of fruit;
begin 
	c := [red..blue];
	f := [banana];

	c := f;					{error: incompatible set types}
    if c = f then			{error: incompatible set types}
		writeln('c = f.');
	if c <> f then			{error: incompatible set types}
		writeln('c <> f.');
	if c <= f then			{error: incompatible set types}
		writeln('c <= f.');	
	if c >= f then			{error: incompatible set types}
		writeln('c >= f.');
	c := c + f;				{error: incompatible set types}
	c := c * f;				{error: incompatible set types}
	c := c - f;				{error: incompatible set types}
		
	if green in f then		{error: incompatible set types}
		writeln('green in fruit.');
	
	if apple in c then		{error: incompatible set types}
		writeln('apple in color.')		
end.
