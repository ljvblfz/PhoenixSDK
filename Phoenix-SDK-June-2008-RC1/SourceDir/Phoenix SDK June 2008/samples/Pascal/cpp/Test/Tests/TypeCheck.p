{ Tests Pascal type checking. }

program TypeCheck;
const c1 = 1; c2 = 2.0;
var
   X,D: real;
   Y,C: integer;
   A,B,A: integer;	{ error: 'A' is already declared }
   b: Boolean;
   c: Char;
begin A := bar;		{ error: 'bar' is undeclared }
	A := 5 DIV 5;
	b := true;
	X := 3.14;
	X := 4;			{ ok: assigment of integer to real }
	Y := 3.14;		{ ok: assigment of real to integer }
	Y := 4;
	C := c1;
	C := c2;
	D := c1;
	D := c2;	
	Z := 2;			{ error: 'Z' is undeclared }
	c1 := 2;		{ error: assignment to const }
	A := 0;
	B := A;
	C := 0.0;
	c := 'A';	
	b := 5;			{ error: assigning integer to boolean }
	A := true;		{ error: assigning boolean to integer }
	B := false;		{ error: assigning boolean to integer }
	A := 'A';		{ error: assigning char to integer }
	c := 5			{ error: assigning integer to char }
end.