{ Tests the Pascal Char type. }

program Char1(output);

var a,b,c,d : Char;
	w,x,y,z : Integer;

begin

	a := '*';
	b := 'G';
	c := '''';
	d := 'x';
	
	writeln(a,b,c,d);
	
	w := ord(a);
	x := ord(b);
	y := ord(c);
	z := ord(d);
	
	writeln(w,',',x,',',y,',',z);
	
	a := chr(w);
	b := chr(x);
	c := chr(y);
	d := chr(z);
	
	writeln(a,b,c,d);
	
	{ Test relational operators }
	writeln(a,' = ',b,' = ', a=b);
	writeln(c,' <> ',d,' = ', c<>d);
	writeln(a,' < ',b,' = ', a<b);
	writeln(c,' <= ',d,' = ', c<=d);
	writeln(a,' >= ',b,' = ', a>=b);
	writeln(c,' > ',d,' = ', c>d);
	
	writeln;
	
	{ Test transfer functions }	
	
	a := chr(567);
	writeln('chr(',567,') = ', a);
	writeln('ord(',a,') = ', ord(a));
	
	writeln('pred(','c',') = ', pred('c'));
	writeln('succ(','c',') = ', succ('c'));

	writeln('sqrt(',ord('a'),') = ', sqrt(ord('a')))
end.