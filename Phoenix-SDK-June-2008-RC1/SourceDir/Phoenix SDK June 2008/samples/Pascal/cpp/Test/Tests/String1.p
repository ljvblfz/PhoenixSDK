{ Tests the Pascal 'String' type. }

program String1(output);

const n = 10;
type String10 = packed array[1..n] of char;
var a,b,c,d : String10;

begin

	a := '**        ';
	b := 'GG        ';
	c := '''''        ';
	d := 'xx        ';
	
	writeln(a,b,c,d);
	writeln('a=','''',a,'''');
	writeln('b=','''',b,'''');
	writeln('c=','''',c,'''');
	writeln('d=','''',d,'''');
	
	writeln;
		
	{ Test relational operators }
	
	writeln('a = b',' = ', a=b);
	writeln('c <> d',' = ', c<>d);
	writeln('a < b',' = ', a<b);
	writeln('c <= d',' = ', c<=d);
	writeln('a >= b',' = ', a>=b);
	writeln('c > d',' = ', c>d);

	writeln;
	
	{ Test transfer functions }	
	
	pack(a,0,b);
	unpack(c,d,0);
	
	writeln(a,b,c,d);
	writeln('a=','''',a,'''');
	writeln('b=','''',b,'''');
	writeln('c=','''',c,'''');
	writeln('d=','''',d,'''')
end.