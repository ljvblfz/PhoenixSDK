{ Test Pascal subrange types. }

program Subrange2;
type
	real_subrange = 1.5..5.0; {error: subrange must be ordinal}
var
	r : real_subrange; {error: type 'real_subrange' not found}
begin
	r := 2.0
end.