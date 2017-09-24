{ Test the Pascal With statement. }

program With4(output);

type complex = record re,im: real
			  end;
var px : ^complex;
begin
	new(px);
	
	with px^ do
		begin re := 6; im := 7
		end; {with}
		
	with px^ do
		writeln('px = ', re:3, im:3);
		
	dispose(px)
end.