{ Test Pascal sets.
 generate the primes between 2..512 using a 
 sieve containing odd integers in this range. }

program Sieve(output);

const n = 256;
var	sieve,primes : set of 2..n;
	next,j,c : integer;
begin {initialize}	
	sieve := [2..n]; primes := []; next := 2;
	writeln(next);
	repeat {find next prime}
		while not(next in sieve) do next := succ(next);
		primes := primes + [next];
		c := 2*next - 1; {c = new prime}
		writeln(c);
		j := next;
		while j<=n do {eliminate}
			begin sieve := sieve - [j]; j := j+c
			end;
	until sieve=[]
end.