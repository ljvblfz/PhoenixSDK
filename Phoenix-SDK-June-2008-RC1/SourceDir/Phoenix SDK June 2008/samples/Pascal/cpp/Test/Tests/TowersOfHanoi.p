{Solves the TowersOfHanoi problem by using
 a recursive algorithm.}
 
program TowersOfHanoi(hanoi,output);

const left = 'A'; middle = 'B'; right = 'C';
	minheight = 1; maxheight = 31;
var height, moves, format : integer;
	towers : array[left..right] of set of minheight..maxheight;
	hanoi : file of integer;
	
procedure PrintTowers;
var c : char; i : integer;
	procedure PrintDisk(disk : integer; empty : Boolean);
	begin
		if not empty then
		begin
			if disk < 10 then
				write('  *',disk,'*   ')
			else
				write(' **',disk,'**  ')
		end
		else write('         ')
	end;
begin
	for i := height downto minheight do
	begin
		for c := left to right do
		begin
			if i in towers[c] then PrintDisk(height-i+1, false)
  		    else PrintDisk(height-i+1, true)
		end;
		writeln;
	end;
	for c := left to right do write('======== ')
end;

procedure Hanoi(height : integer; source, dest, using : char);
	procedure MoveDisk(disk : integer; source, destination : char);
	begin 
		moves := moves + 1;
		towers[source] := towers[source] - [disk];
		towers[destination] := towers[destination] + [disk]
	end;
begin
	if height >= 1 then 
	begin
         Hanoi(height-1, source, using, dest); 
         MoveDisk(height, source, dest);
         Hanoi(height-1, using, dest, source);
	end
end;

begin moves := 0;
	towers['A'] := []; 
	towers['B'] := [];
	towers['C'] := [];	
	read(hanoi, height);	
	if (height > maxheight) or (height < minheight) then
		writeln(height, ': invalid height.')
	else begin
		towers['A'] := [1..height];
		writeln('Before:'); PrintTowers;
		Hanoi(height, left, middle, right);
		writeln; writeln('Solved in ', moves, ' moves.');
		writeln('After:'); PrintTowers
	end
end.