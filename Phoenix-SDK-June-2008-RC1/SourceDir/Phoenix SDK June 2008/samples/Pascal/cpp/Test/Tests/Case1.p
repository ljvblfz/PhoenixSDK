{ Test Pascal Case statements. }

program Case1(output);
type string10 = packed array[1..10] of char;
var
	s : string10;
	d,a,x : integer; {counts digit,alpha,other}

procedure CountChars(var s : string10; var d,a,x : integer);
var i : integer;
begin
	for i := 1 to 10 do 
	begin
		case s[i] of
			'0'..'9': d := d + 1;
			'A'..'Z': a := a + 1;
			'a'..'z': a := a + 1;
			otherwise x := x + 1
		end
	end
end;

begin
	s := 'cAiy:2lSi5';
	d := 0; a := 0;	x := 0;
	
	CountChars(s,d,a,x);
	writeln(s, ' contains ', d, ' digit, ', a,' alpha, and ',
		x, ' other characters.')
end.