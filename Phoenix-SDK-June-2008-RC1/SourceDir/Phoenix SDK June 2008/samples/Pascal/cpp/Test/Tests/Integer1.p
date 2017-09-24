{ Tests the Pascal Integer type. }

program Integer1(output);

const pi = 3.14159;
var i,j: Integer;

begin
	
	i := 26;
	j := 6;
	
	{ Print the built-in 'maxint' constant }
	writeln('maxint = ', maxint);
	
	writeln;
	
	{ Test arithmetic operators }
	writeln(i,' * ',j,' = ', i*j);
	writeln(i,' div ',j,' = ', i div j);
	writeln(i,' mod ',j,' = ', i mod j);
	writeln(i,' + ',j,' = ', i + j);
	writeln(i,' - ',j,' = ', i - j);
	
	writeln;
	i := i + j;
	j := i - j;
	i := i - j;	
	
	writeln(i,' * ',j,' = ', i*j);
	writeln(i,' div ',j,' = ', i div j);
	writeln(i,' mod ',j,' = ', i mod j);
	writeln(i,' + ',j,' = ', i + j);
	writeln(i,' - ',j,' = ', i - j);
		
	writeln;
	i := i + j;
	j := i - j;
	i := i - j;	
	
	{ Test relational operators }
	writeln(i,' = ',j,' = ', i=j);
	writeln(i,' <> ',j,' = ', i<>j);
	writeln(i,' < ',j,' = ', i<j);
	writeln(i,' <= ',j,' = ', i<=j);
	writeln(i,' >= ',j,' = ', i>=j);
	writeln(i,' > ',j,' = ', i>j);
	
	writeln;
	i := i + j;
	j := i - j;
	i := i - j;	
	
	writeln(i,' = ',j,' = ', i=j);
	writeln(i,' <> ',j,' = ', i<>j);
	writeln(i,' < ',j,' = ', i<j);
	writeln(i,' <= ',j,' = ', i<=j);
	writeln(i,' >= ',j,' = ', i>=j);
	writeln(i,' > ',j,' = ', i>j);
	
	writeln;
	
	{ Test some Standard Integer functions }
	writeln('abs(',i,') = ', abs(i));
	writeln('abs(',-i,') = ', abs(-i));
	writeln('-abs(',i,') = ', -abs(i));
	
	writeln('sqr(',i,') = ', sqr(i));
	writeln('sqr(',j,') = ', sqr(j));
	writeln('trunc(',pi,') = ', trunc(pi));
	writeln('trunc(',pi/2,') = ', trunc(pi/2));
	writeln('round(',pi,') = ', round(pi));
	writeln('round(',pi/2,') = ', round(pi/2))	
end.