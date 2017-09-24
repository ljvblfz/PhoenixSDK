{ Tests invalid function/procedure invocations. }

program BadCall;
var i : integer;

procedure p;
	procedure p1;
	begin
		f1; { error: undeclared (not yet declared) procedure or function. }
		f2; { error: undeclared (not yet declared) procedure or function. }
		p   { call parent }
	end;
	
	function f1: integer;
	begin
		p1; { call sibling }
		p2; { error: undeclared (not yet declared) procedure or function. }
		p   { call parent }
	end;
begin
	p1; f1  { call children }
end;

function f: integer;
	procedure p2;
	begin 
		f2; { error: undeclared (not yet declared) procedure or function. }
		f1; { error: wrong scope }
		f   { call parent }
	end;
	
	function f2: integer;
	begin
		p2; { call sibling }
		p1; { error: wrong scope }
		f   { call parent }
	end;
begin
	f := 0;
	p2; f2  { call children }	
end;

begin	

	p; f;	{ these are legal }
	
	p1;		{ error: cannot call p's inner functions }
	i := f1;{ error: wrong scope }
	
	p2;		{ cannot call f's inner functions }
	i := f2 { error: wrong scope }
end.