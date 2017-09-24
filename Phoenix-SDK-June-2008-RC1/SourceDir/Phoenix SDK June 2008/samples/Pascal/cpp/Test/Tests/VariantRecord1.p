{ Test Pascal variant record declarations. }

program VariantRecord1(output);

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
var p : person;
begin  	
	p.name.last := 'woodyard  ';
	p.name.first := 'edward    ';
	p.ss := 845680539;
	p.sex := male;
	p.birth.mo := aug;
	p.birth.day := 30;
	p.birth.year := 1941;
	p.depdts := 1;
	p.ms := divorced;		
		p.ddate.mo := sept;
		p.ddate.day := 19;
		p.ddate.year := 1977;
		p.firstd := true;

	writeln('first:  ',p.name.first);
	writeln('last:   ',p.name.last);
	writeln('ss:     ',p.ss);
	writeln('sex:    ',ord(p.sex));	
	writeln('birth:  ',ord(p.birth.mo)+1,'/',p.birth.day,'/',p.birth.year);
	writeln('depdts: ',p.depdts);
	writeln('ms:     ',ord(p.ms));
	writeln('ddate:  ',ord(p.ddate.mo)+1,'/',p.ddate.day,'/',p.ddate.year);
	writeln('firstd: ',p.firstd)
end.