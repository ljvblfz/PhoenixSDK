{ Tests the Pascal Real type. }

program Real1(output);

const pi = 3.14159;
var i,j: Integer;
	r,s: Real;

begin
	
	i := 52;
	j := 3;
	
	r := 3.5;
	s := -1.234;
	
	{ Test arithmetic operators }
	writeln(r,' * ',s,' = ', r*s);
	writeln(r,' * ',i,' = ', r*i);
	writeln(j,' * ',s,' = ', j*s);
	
	writeln(r,' / ',s,' = ', r/s);
	writeln(r,' / ',i,' = ', r/i);
	writeln(i,' / ',j,' = ', i/j);
	
	writeln(r,' + ',s,' = ', r+s);
	writeln(r,' + ',i,' = ', r+i);
	writeln(r,' - ',s,' = ', r-s);
	writeln(s,' - ',r,' = ', s-r);
	writeln(s,' - ',j,' = ', s-j);
	
	writeln;
	
	{ Test relational operators }
	writeln(s,' = ',r,' = ', s=r);
	writeln(s,' <> ',r,' = ', s<>r);
	writeln(s,' < ',r,' = ', s<r);
	writeln(s,' <= ',r,' = ', s<=r);
	writeln(s,' >= ',r,' = ', s>=r);
	writeln(s,' > ',r,' = ', s>r);
	
	writeln(s,' = ',i,' = ', s=i);
	writeln(s,' <> ',j,' = ', s<>j);
	writeln(s,' < ',i,' = ', s<i);
	writeln(s,' <= ',j,' = ', s<=j);
	writeln(s,' >= ',i,' = ', s>=i);
	writeln(s,' > ',j,' = ', s>j);
	
	writeln;
	
	{ Test some Standard Real functions }
	writeln('abs(',pi,') = ', abs(pi));
	writeln('abs(',-pi,') = ', abs(-pi));
	writeln('-abs(',pi,') = ', -abs(pi));
	
	writeln('sqr(',pi,') = ', sqr(pi));
	writeln('sin(',pi/2,') = ', sin(pi/2));
	writeln('cos(',pi/2,') = ', cos(pi/2));
	writeln('sin(',2,') = ', sin(2));
	writeln('cos(',2,') = ', cos(2));
	writeln('arctan(',5.0,') = ', arctan(5.0));
	writeln('arctan(',5,') = ', arctan(5));
	writeln('ln(',r,') = ', ln(r));
	writeln('ln(',i,') = ', ln(i));
	writeln('exp(',2.302585,') = ', exp(2.302585));
	writeln('exp(',j,') = ', exp(j));	
	writeln('sqrt(',45.35,') = ', sqrt(45.35));
	writeln('sqrt(',i,') = ', sqrt(i))
end.