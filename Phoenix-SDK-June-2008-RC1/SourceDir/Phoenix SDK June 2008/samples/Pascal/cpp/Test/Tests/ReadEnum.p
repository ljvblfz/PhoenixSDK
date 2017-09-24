{ Test errors reading enumerated types. }

program ReadEnum(input,output);

type color = (red,green,blue);
var b : Boolean; c : color;
begin	
	read(b);		{error: cannot read enum from input}
	writeln(b);
	
	read(c);		{error: cannot read enum from input}
	writeln(c)		{error: cannot write enum to output}
end.