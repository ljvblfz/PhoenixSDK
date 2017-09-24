{ Tests the Pascal 'String' type. }

program String3(output);

type String10 = packed array[1..10] of char;
	 String15 = packed array[1..15] of char;
	 String20 = packed array[1..20] of char;
var a: String10;
	b: String15;
	c: String20;	
begin

	a := 'abcdefghij';
	b := '123456789012345';
	c := '1b3d5f7h9j1l3n5p7r9t';

	{ Test relational operators }
	
	writeln('abc',' = ','abc',' = ', 'abc'='abc');
	
	writeln(a,' = ','abc',' = ', a='abc');   {error: mismatched string types}
	writeln(c,' <> ','abc',' = ', c<>'abc'); {error: mismatched string types}
	writeln(a,' < ','abc',' = ', a<'abc');	 {error: mismatched string types}
	writeln(c,' <= ','abc',' = ', c<='abc'); {error: mismatched string types}
	writeln(a,' >= ','abc',' = ', a>='abc'); {error: mismatched string types}
	writeln(c,' > ','abc',' = ', c>'abc')    {error: mismatched string types}
end.