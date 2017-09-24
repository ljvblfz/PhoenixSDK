{ Tests an invalid variable declaration. }

program BadVarDecl(output);

var a: Integer;
	a: Real;	{ error: 'a' already declared }

begin	
	a := 52;
	a := 3.0;
end.