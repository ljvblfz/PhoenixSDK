{ Test the Pascal With statement. }

program With1(output);

type complex = record re,im: real
			  end;
var x : complex;
begin  
		
	{Open and Closed With}	

	with x do
		begin re := 6; im := 7
		end; {with}
	
	with x do
		if re < 1 then re := 1;
		{with}

		
	with x do
		writeln('x = ', re:3, im:3)
end.