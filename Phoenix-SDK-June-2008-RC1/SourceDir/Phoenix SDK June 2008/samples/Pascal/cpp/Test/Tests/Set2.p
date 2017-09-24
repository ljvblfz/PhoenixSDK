{ Test Pascal sets. }

program Set2(output);

var ch : char;
	chset1, chset2, chset3: set of 'a'..'z';
begin   
	
	ch := 'q';		
	chset1 := ['d','a','g'];
	chset3 := ['a'..'n'];
	chset2 := ['a'..'z']-[ch];	
	
	{set (in)equality}
	
	write('chset3 = chset2?: ');
	if chset3 = chset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('chset3 <> chset2?: ');
	if chset3 <> chset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('[a..n] = [a..n]?: ');
	if ['a'..'n'] = ['a'..'n'] then
		writeln('yes')
	else
		writeln('no');
	
	write('[a..n] <> [a..n]?: ');
	if ['a'..'n'] <> ['a'..'n'] then
		writeln('yes')
	else
		writeln('no');
		
	{set inclusion}
	
	write('chset3 subset of chset2?: ');
	if chset3 <= chset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('chset3 superset of chset2?: ');
	if chset3 >= chset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('chset1 subset of chset2?: ');
	if chset1 <= chset2 then
		writeln('yes')
	else
		writeln('no');
		
	write('chset1 superset of chset2?: ');
	if chset1 >= chset2 then
		writeln('yes')
	else
		writeln('no');
		
	writeln;		
	
	{set difference}
	write('[a..z]-[',ch,']: [');
	for ch := 'a' to 'z' do
		begin
			if ch in chset2 then write(ch);
		end;
	writeln(']');
	
	{set intersection}
	chset2 := chset1*chset2;
	write('[d,a,g]*([a..z]-[',ch,']): [');
	for ch := 'a' to 'z' do
		begin
			if ch in chset2 then write(ch);
		end;		
	writeln(']');
	
	{set union}
	chset2 := chset2+['a'..'n'];
	write('([a..z]-[',ch,'])+[a..n]: [');
	for ch := 'a' to 'z' do
		begin
			if ch in chset2 then write(ch);
		end;
	writeln(']')
end.
