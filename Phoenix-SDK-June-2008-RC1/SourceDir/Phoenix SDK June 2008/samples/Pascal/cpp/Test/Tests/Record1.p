{ Test Pascal record declarations. }

program Record1(output);
type
	complex = record re,im: real
			  end;

	date = record mo: (jan,feb,mar,apr,may,june,
						july,aug,sept,oct,nov,dec);
				  day: 1..31;
				  year: integer
		   end;
		   
	toy = record kind:(ball,top,boat,doll,blocks,
					   game,model,book);
				 cost: real;
				 received: date;
				 enjoyed: (alot,some,alittle,none);
				 broken,lost: Boolean;
		  end;
		  
	assignment = record subject:(history,language,list,
								 math,psych,science);
						assigned: date;
						grade: 0..4;
						weight: 1..10;
				 end;
	
	family = (father, mother, child1, child2, child3);

var
	x : complex;
	vaccine : array[family] of date;
begin   

	vaccine[child3].mo := apr;
	vaccine[child3].day := 23;
	vaccine[child3].year := 1973;
	
	writeln(ord(vaccine[child3].mo)+1,'/',
			vaccine[child3].day,'/',
			vaccine[child3].year)
	
end.