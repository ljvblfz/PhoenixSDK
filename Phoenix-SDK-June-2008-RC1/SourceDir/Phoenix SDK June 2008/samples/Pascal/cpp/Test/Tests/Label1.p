{ Test Pascal labels. }

program Label1(lbl,output);
label 111, 222, 333, 888, 999;
var	x : integer;
	lbl : file of integer;
begin 
repeat
goto 222;
333: read(lbl,x);
	if x = 888 then goto 888
	else if x = 999 then goto 999
	else 
	begin writeln(x,': not 888 nor 999...'); goto 111 
	end;
222: goto 333;
888: writeln(x);
	goto 111;
999: writeln(x);
111: writeln('111');
until x < 0
end.