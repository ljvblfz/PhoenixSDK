{ Test Pascal pointer types. }

program Pointer3(output);
var a,b,c,d : ^integer;
begin 
	new(a); new(b); new(c); new(d);
	a^ := 1;
	b^ := 2;
	c^ := 3;
	d^ := 4;
	writeln(a^);
	writeln(b^);
	writeln(c^);
	writeln(d^);
	dispose(a); dispose(b); dispose(c); dispose(d);
	
	{ test for pointer equivalence }
	new(a); new(b);
	a^ := 1;
	b^ := 2;
	c := a;
	d := b;
	if a = a then writeln('a=a');
	if a = b then writeln('a=b');
	if a = c then writeln('a=c');
	if a = d then writeln('a=d');
	if b = a then writeln('b=a');
	if b = b then writeln('b=b');
	if b = c then writeln('b=c');
	if b = d then writeln('b=d');
	if c = a then writeln('c=a');
	if c = b then writeln('c=b');
	if c = c then writeln('c=c');
	if c = d then writeln('c=d');
	if d = a then writeln('d=a');
	if d = b then writeln('d=b');
	if d = c then writeln('d=c');
	if d = d then writeln('d=d');	
	dispose(a); dispose(b)
end.