{ Test the Pascal With statement. }

program With6(output);

type T = record b: Boolean
			    end;
var t : T; b : Boolean;
begin	
	b := true;
	t.b := false;
		
	with t do
		writeln('t.b = ', b);
	writeln('b = ', b)
end.