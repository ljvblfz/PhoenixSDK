{ Test Pascal labels. }

program Label3;
label 111, 222;
begin
111: { warning: never referenced }
goto 654; { error: undeclared label }
654: { error: undeclared label }
{ warning: 222 never referenced }
end.