{ Test Pascal subrange types. }

program Subrange1(output);
type
	days = (mon,tues,wed,thur,fri,sat,sun); {scalar type}
	index = 0..63; {subrange of integer}
	letter = 'a'..'z'; {subrange of char}
var
	workday,weekend : set of days;
	a : 1..10; b : 0..30; c : 20..30;
begin

	workday := [mon..fri];
	weekend := [sat..sun];
	
	a := 6;
	c := 25;
	b := 10;
	
	a := b;
	b := c;
	
	writeln(a,',',b,',',c)
	
end.