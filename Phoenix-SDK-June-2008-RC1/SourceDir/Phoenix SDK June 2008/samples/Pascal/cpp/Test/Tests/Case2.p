{ Test Pascal Case statements. }

program Case2(output);

var i,n : integer;
begin
	case i of
		10..20: n := 10;
		15..25: n := 15;
		20..30: n := 20;
		otherwise n := 0;
	end
end.