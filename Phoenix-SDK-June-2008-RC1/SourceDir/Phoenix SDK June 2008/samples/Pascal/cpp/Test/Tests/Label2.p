{ Test Pascal labels. }

program Label2(output);
label 999;
begin 
goto 999;
999: writeln('first');
999: writeln('second') { error: label redefinition }
end.