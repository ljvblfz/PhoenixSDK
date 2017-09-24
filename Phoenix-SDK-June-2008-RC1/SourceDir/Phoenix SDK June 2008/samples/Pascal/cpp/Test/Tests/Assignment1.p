{ Tests assignment statements. }

program Assignment1(output);

const pi = 3.14159;
var root1,root2,root3,x,y,z: Real;
    found: Boolean;
    count,degree,sqrpr,pr: Integer;
begin	
	x := pi/2.0;
	y := pi/3.0;
	z := pi*2.0;
	root1 := pi*x/y;
	root2 := -root1;
	root3 := (root1 + root2)*(1.0 + y);
	found := y > z;
	count := 4;
	count := count + 1;
	degree := -10;
	degree := degree + 10;
	pr := 3;
	sqrpr := sqr(pr);
	y := sin(x) + cos(y);

	writeln(root1);
	writeln(root2);
	writeln(root3);
	writeln(x);
	writeln(y);
	writeln(z);
	writeln(found);
	writeln(count);
	writeln(degree);
	writeln(sqrpr);
	writeln(pr)
end.