{ Tests the operator precedence. }

program OperatorPrecedence(output);

begin	
	writeln(2 * 3-4 * 5); {-14}
	writeln(15 div 4 * 4); {-12}
	writeln(80/5/3); {5.333}
	writeln(4/2 * 3); {6.000}
	writeln(sqrt(sqr(3) + 11*5)) {8.000}
end.