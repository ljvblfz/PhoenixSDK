{ Test Pascal parameter passing. }
 
program Parameters3(output);

type complex = record re, im : integer end;
	 arrayOfInteger = array[1..10] of integer;
var x, y : complex; a, b : arrayOfInteger;
	i : integer;
	
    procedure p(c1 : complex; var c2 : complex;
		var a1 : arrayOfInteger; var a2 : arrayOfInteger);		
    begin
		c1.re := c1.im + 13; c1.im := c1.re + 14; 
		c2.re := c2.im + 17; c2.im := c2.re + 18; 
		
		for i := 1 to 10 do
		begin
			a1[i] := a1[i] * 2;
			a2[i] := a2[i] * 2
		end
    end;

    procedure printComplex(var c : complex);    
    begin
		writeln(c.re:2,' ',c.im:2)
    end;
    
    procedure printArray(var a : arrayOfInteger);
    var i : integer;
    begin
		for i := 1 to 10 do
			write(a[i]:2, ' ');
		writeln
    end;
    
begin 
    x.re := 3; x.im := 4; 
	y.re := 7; y.im := 8; 
	
	for i := 1 to 10 do
	begin
		a[i] := 6 + i;
		b[i] := 8 + i
	end;
	
	printComplex(x); printComplex(y);
	printArray(a); printArray(b);

	p(x,y, a,b);
	
	printComplex(x); printComplex(y);
	printArray(a); printArray(b)
end.