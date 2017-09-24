{ program 11.4
 conversion to postfix form }
 
program Postfix(postfix, output);

var ch : char; postfix : text;

procedure find;
begin repeat readln(postfix, ch)
	until (ch<>' ') and not eoln(postfix)
end;

procedure expression;
	var op : char;
		
	procedure term;
		
	procedure factor;
	begin if ch='(' then
		begin find; expression; {ch = ) }
		end else write(ch);
		find
	end; {factor}
		
	begin factor;
		  while ch='*' do
		  begin find; factor; write('*')
		  end
	end; {term}
		
begin term;
	  while (ch='+')or(ch='-') do
		begin	op := ch; find; term; write(op)
		end
end; {expression}

begin find;
	repeat write(' ');
		expression;
		writeln
	until ch='.'
end.