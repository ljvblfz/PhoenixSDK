{ Test Pascal parameter passing. }
 
program Parameters2(output);

procedure print(r:real);
begin writeln(r)
end; {print}

procedure something(procedure p(q:real); r: real);
begin p(r)
end; {something}

function trig(function f(q:real): real; x: real): real;
begin trig := f(x)
end; {trig}

begin {main}
	something(print, trig(sin, 1.570796));
	something(print, trig(cos, 1.570796))
end.