{ program 11.8
 extend program 4.8 }

program Expon2(output);

var	pi,spi: real;

function power(x:real; y:integer): real; {y>=0}
	var z: real;
begin z := 1;
	while y>0 do
	begin
		while not odd(y) do
		begin y := y div 2; x := sqr(x)
		end;
	y := y-1; z := x*z
	end;
	power := z
end; {power}

begin pi := 3.14159;
	writeln(2.0,'\t',7,'\t',power(2.0,7));
	spi := power(pi,2);
	writeln(pi,'\t',2,'\t',spi);
	writeln(spi,'\t',2,'\t',power(spi,2));
	writeln(pi,'\t',4,'\t',power(pi,4))
end.