{ Test pretty-printing.
 This is the Sieve program, with added whitespace.
 Compiling this with /p will produce a pretty-printed
 version of the source program. }

program Ugly			(output);

const           n
  
  =
   256;
var	sieve,					primes 

: set of 2..n;
next,j,c : integer;
begin 


{initialize}	



	sieve:=[2..n];primes:=[];next:=2            ;writeln(next);
	repeat {find next prime}
		while not(next					in 
		sieve) 
		
		
		do next := succ(next);		primes := primes + [next];	
			c := 2*next - 1; {c = new prime}
		writeln(			c)	;
		j := next	;
while j<=n do {eliminate}
begin sieve := sieve -			[j]; j := j				+			c
end;
until sieve=[]

end.