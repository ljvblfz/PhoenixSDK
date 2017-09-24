{ Tests the Pascal 'String' type. }

program String2(output);

type String10 = packed array[1..10] of char;
	 String15 = packed array[1..15] of char;
	 String20 = packed array[1..20] of char;
var a: String10;
	b: String15;
	c: String20;
	d: String20;
begin

	a := '1234567890';
	b := '123456789012345';
	c := '12345678901234567890';
	d := '123';	{error; mismatched string types}

	{ Test relational operators }
	
	writeln('a = b',' = ', a=b);	{error; mismatched string types}
	writeln('c <> d',' = ', c<>d);
	writeln('a < b',' = ', a<b);	{error; mismatched string types}
	writeln('c <= d',' = ', c<=d);
	writeln('a >= b',' = ', a>=b);	{error; mismatched string types}
	writeln('c > d',' = ', c>d);
	
	writeln;
	
	{ Test transfer functions }	
	
	pack(a,0,b);	{error; mismatched string types}
	unpack(c,d,0)	
end.