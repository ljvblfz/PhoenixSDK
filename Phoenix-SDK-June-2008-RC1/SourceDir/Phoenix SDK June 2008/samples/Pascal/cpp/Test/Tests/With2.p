{ Test the Pascal With statement. }

program With2(output);

type alfa = packed array[1..10] of char;
	 status = (married,widowed,divorced,single);
	 date = record mo : (jan,feb,mar,apr,may,jun,
						july,aug,sept,oct,nov,dec);
			   day : 1..31;
			   year : integer;
			end;
	person = record
				name : record first,last : alfa 
					   end;
				ss : integer;
				sex : (male, female);
				birth : date;
				depdts : integer;
				case ms : status of
					married, widowed : (mdate : date);
					divorced : (ddate : date;
								firstd : Boolean);
					single : (indepdt : Boolean)
			 end; {person}
var p : person; o  : integer;
begin  
	with p,name,birth do
	begin last := 'woodyard  ';
		  first := 'edward    ';
		  ss := 845680539;
		  sex := male;
		  mo := aug;
		  day := 30;
		  year := 1941;
		  depdts := 1;
		  ms := divorced;		
		  ddate.mo := sept;
		  ddate.day := 19;
		  ddate.year := 1977;
		  firstd := true
	end;{with}
	
	with p,name,birth do
	begin				
		writeln('first:  ',first);
		writeln('last:   ',last);
		writeln('ss:     ',ss);
		writeln('sex:    ',ord(sex));
		writeln('birth:  ',ord(mo)+1,'/',day,'/',year);
		writeln('depdts: ',depdts);
		writeln('ms:     ',ord(ms));
		writeln('ddate:  ',ord(ddate.mo)+1,'/',ddate.day,'/',ddate.year);
		writeln('firstd: ',firstd)
	end {with}
end.