{ Test runtime range checking. }

program RuntimeRangeCheck(output);
var
	day : 1..31;
begin
	day := 1;
	while day < 100 do 
		day := day + 1 { should runtime error at day = 32 }
end.