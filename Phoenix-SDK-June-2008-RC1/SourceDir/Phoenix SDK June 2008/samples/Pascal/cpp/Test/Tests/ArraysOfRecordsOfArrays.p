{ Test arrays of record types, which in turn contain arrays. }

program ArraysOfRecordsOfArrays(output);
type
	C = record 
			k : Integer;
			n : array[1..4] of Integer;
			m : array[1..2, 1..2] of Integer
		end;
var
	c : array[1..2] of C;
begin   
	
	c[1].k := 88;		
	writeln(c[1].k);
	
	c[2].m[1,1] := 55;
	writeln(c[2].m[1,1])
end.