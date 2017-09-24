{ Test the Pascal While statement. }

program While1(Output);
var X : Integer;
begin
	{Closed While}
	X := 0;	While X < 10 Do
	begin
		Writeln(X);
		X := X + 1
	end;
		
	{Open While}
	X := 0;	While X < 10 Do
		if X < 100 then 
			X := X + 100;
	Writeln(X)
end.