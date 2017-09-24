{ Test Pascal sets. }

program Set1(output);

type primary = (red, yellow, blue);
	 color = set of primary;
var	ch : char;
	chset : set of 'a'..'y';
	hue1, hue2 : color;
begin   	
	hue1 := [red..yellow];
	hue2 := [];
	hue2 := hue2 + [succ(red)];
	
	writeln('Members of hue1:');
	if red in hue1 then
		writeln('* red');
	if yellow in hue1 then
		writeln('* yellow');
	if blue in hue1 then
		writeln('* blue');
		
	writeln('Members of hue2:');
	if red in hue2 then
		writeln('* red');
	if yellow in hue2 then
		writeln('* yellow');
	if blue in hue2 then
		writeln('* blue');
	
	chset := ['d','a','g'];
	writeln('Members of chset:');
	for ch := 'a' to 'z' do 
	begin
		if ch in chset then
			writeln('* ', ch)
	end
end.