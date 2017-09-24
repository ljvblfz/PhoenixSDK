{ Tests invalid label declarations. }

program BadLabelDecl;

label 333, 67890; { error: labels can contain at most 4 digits }
begin	
	{ The compiler should warn here as well, since the labels are never 
	  defined or referenced. }
end.