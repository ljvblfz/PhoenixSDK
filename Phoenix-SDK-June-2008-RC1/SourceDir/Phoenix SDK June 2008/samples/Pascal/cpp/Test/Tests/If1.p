{ Test Pascal conditional statements. }

program If1(iffile, Output);
var W,X,Y: Integer;
	iffile: File of Integer;
begin W := 0; Y := 2;
	Reset(iffile);
	While Not Eof(iffile) Do
	Begin
		Read(iffile, X);
	
		If (W <> 0) And ((Y = 2) Or (W = 0)) Then Writeln(true)
		Else Writeln(false);
		
		If ((W <> 0) And (Y = 2)) Or (W = 0) Then Writeln(true)
		Else Writeln(false);
		
		{ Open if }
		If X < 2 Then 
		Begin
			W := W + X + Y;	W := W + X + Y;
			W := W + X + Y; W := W + X + Y;
			W := W + X + Y
		End
		Else If X < 4 Then	W := 2
		Else				W := 66;
		
		Writeln(W);
	
		{ Closed if }
		If X < 2 Then 		W := X + Y
		Else				W := 2;
		Writeln(W)	
	end
end.