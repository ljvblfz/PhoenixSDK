{ Test Pascal labels. }

program Label4;
label 10,100,100,1000,10000; { error: redeclaration of label '100' }
							 { error: label '10000' out of range }
begin
goto 10;
10: goto 100;
100: goto 1000;
1000: goto 10000;
10000:
end.