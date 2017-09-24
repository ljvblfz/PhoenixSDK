{ Test Pascal parameter passing. }
 
program Parameters4(output);

const n = 50;
type arrayOfInteger = array[1..10] of integer;
	 integerFile = file of integer;
	 string10 = array[1..10] of char;	 
var a : arrayOfInteger; s : string10;
	tempFile : integerFile;
	
    procedure p(a : arrayOfInteger; s : string10); { error: must pass arrays and strings by reference }
    begin
		a[1] := 1; s := '0123456789'
    end;
    
    procedure q(var i : integer);
    begin
		i := i + 1
    end;
    
    procedure writeInteger(i : integer; f : integerFile); { error: must pass files by reference }
    begin
		write(f, i);
    end;
    
begin 
	writeInteger(41, tempFile);
	p(a, s);
	q(n); { error: pass by reference parameters must be non-const }
	q(3)  { error: pass by reference parameters must be non-const }
end.