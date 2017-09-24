{ Test static (compile-time) range checking. }

program StaticRangeCheck(output);
var
	day : 1..31;
begin
	day := 0;	{ error: out of range }
	day := 1;	{ ok. }
	day := 2;	{ ok. }
	day := 30;	{ ok. }
	day := 31;	{ ok. }
	day := 32	{ error: out of range }
end.