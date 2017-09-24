{ Test Pascal record declarations. }

program Record2(output);
type rec = record x : rec; end; { error: recursive definition }
var r : rec;
begin   
end.