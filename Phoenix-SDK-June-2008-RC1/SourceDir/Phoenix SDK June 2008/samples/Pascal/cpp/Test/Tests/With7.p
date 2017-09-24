{ Test the Pascal With statement. }

program With7(output);

type date = record mo : (jan,feb,mar,apr,may,jun,
						july,aug,sept,oct,nov,dec);
			   day : 1..31;
			   year : integer;
			end;
var d : date;
begin  
	{ error: mo, day, and year are invalid with clauses 
	 because they are not aggregate types. }
	with d,mo,day,year do
	begin 
	end;{with}
end.