{ Illustrates the Pascal goto statement. }

program Goto1(output);
label 123;
var	x : integer;
begin 
   x := 3;
   
   {perform an unconditional jump to label 123}
   goto 123;

   {this code is never reached}
   x := x + 100;

123:
   write(x)
end.