{ program 3.1
 example of constant definition part }

program Convert(output);

const addin = 32; mulby = 1.8; low = 0; high = 39;
      separator = '----------';
var degree : low..high;
begin
	writeln(separator);
	for degree := low to high do
	begin write(degree, 'c\t', round(degree*mulby + addin), 'f\t');
		if odd(degree) then writeln
	end;
	writeln;
	writeln(separator)
end.