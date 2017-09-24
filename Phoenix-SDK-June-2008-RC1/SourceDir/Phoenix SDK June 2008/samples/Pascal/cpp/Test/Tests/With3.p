{ Test the Pascal With statement. }

program With3(output);
type date = record mo: (jan,feb,mar,apr,may,june,
						july,aug,sept,oct,nov,dec);
				  day: 1..31;
				  year: integer
		    end;
	
	family = (father, mother, child1, child2, child3);
var
	vaccine : array[family] of date;
begin   

	with vaccine[child3] do 
		begin mo := apr; day := 23; year := 1973
		end;
	
	with vaccine[child3] do 
		writeln(ord(mo)+1,'/',day,'/',year)
	
end.