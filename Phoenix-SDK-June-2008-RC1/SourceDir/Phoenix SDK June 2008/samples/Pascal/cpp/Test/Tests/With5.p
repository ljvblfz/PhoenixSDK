{ Test the Pascal With statement. }

program With5(output);

type T = record x: integer
			    end;
     S = record t: T
			    end;
	 R = record s: S
				end;
var r : R;
begin	
	with r.s.t do
		x := 65; {with}
		
	with r.s.t do
		writeln('r.s.t = ', x)
end.