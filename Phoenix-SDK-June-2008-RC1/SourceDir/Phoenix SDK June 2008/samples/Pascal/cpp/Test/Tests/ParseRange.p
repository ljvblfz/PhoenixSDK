{ Test range parse errors. }

program ParseRange;
var
	X : Integer;
	Y : Real;
begin
	X := 3336402735171707160320; { error: number too large }
	Y := 3336402735171707160320.3336402735171707160320 { error: number too large }
end.