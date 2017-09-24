{ Appendix F from Pascal User Manual and Report, Second Edition
 procedures to read and write real numbers used by the 
 Standard Procedures read(f,x) and write(f,x:n)		 }

program ConvertReal(output);

procedure rdr(var f: text; var x: real);
	{ read real numbers in 'free format' }
	const t48 = 281474976710656.0;
		limit =  56294995342131.0;
		z = 48;			{ ord('0') }
		lim1 = 322;			{ maximum exponent }
		lim2 = -292;		{ minimum exponent }
	type posint = 0..323;
	var ch: char; y: real; a,i,e: integer;
		s,ss: boolean;		{ signs }
		
	function ten(e: posint): real; { = 10**e, 0<e<322 }
		var i: integer; t: real;
	begin i:= 0; t := 1.0;
		repeat if odd(e) then
			case i of
			 0: t := t * 1.0e+1;
			 1: t := t * 1.0e+2;
			 2: t := t * 1.0e+4;
			 3: t := t * 1.0e+8;
			 4: t := t * 1.0e+16;
			 5: t := t * 1.0e+32;
			 6: t := t * 1.0e+64;
			 7: t := t * 1.0e+128;
			 8: t := t * 1.0e+256;
			end ;
			e := e div 2; i := i+1
		until e = 0;
		ten := t
	end ;
	
begin
	{ skip leading blanks }
		while f^=' ' do get(f);
		ch := f^;
		if ch = '-' then
			begin s := true; get(f); ch := f^
			end else
			begin s := false;
				if ch = '+' then
				begin get(f); ch := f^
				end
			end ;
		if not(ch in ['0'..'9']) then
		begin write('**digit expected'); halt;
		end;
		a := 0; e := 0;
		repeat if a < limit then a := 10*a + ord(ch)-z
							else e := e+1;
			   get(f); ch := f^
		until not(ch in ['0'..'9']);
		if ch = '.' then
		begin { read fraction } get(f); ch := f^;
			while ch in ['0'..'9'] do
			begin if a < limit then
				  begin a := 10*a + ord(ch)-z; e := e-1
				  end ;
				get(f); ch := f^
			end
		end ;
		if ch = 'e' then
		begin { read scale factor } get(f); ch := f^;
			i := 0;
			if ch = '-' then
			begin ss := true; get(f); ch := f^
			end else
			begin ss := false; if ch = '+' then
				begin get(f); ch := f^
				end
			end ;
			if ch in ['0'..'9'] then
			begin i := ord(ch)-z; get(f); ch := f^;
				while ch in ['0'..'9'] do 
				begin if i < limit then i := 10*i + ord(ch)-z;
					get(f); ch := f^
				end
			end else
			begin writeln('**digit expected'); halt
			end ;
			if ss then e := e-i else e := e+i;
		end ;
		if e > lim1 then
		begin writeln('**number too large'); halt end;
		{ 0 < a < 2**49 }
		if a >= t48 then y := ((a+1) div 2) * 2.0
					else y := a;
		if s then y := -y;
		if e < 0 then x := y/ten(-e) else
		if e <> 0 then x := y*ten(e) else x := y;
	end;
		
procedure wre(var f: text; x: real; n: integer);
	{write real number x with n characters in decimal flt.pt. format}
	{the following constants are determined by the cdc flt.pt format}
	const t48 = 281474976710656.0; {= 2**48}
		z = 48;			{ ord('0') }
	type posint = 0..323;
	var c,d,e,e0,e1,e2,i: integer;
		
	function ten(e: posint): real; { = 10**e, 0<e<322 }
		var i: integer; t: real;
	begin i:= 0; t := 1.0;
		repeat if odd(e) then
			case i of
			 0: t := t * 1.0e+1;
			 1: t := t * 1.0e+2;
			 2: t := t * 1.0e+4;
			 3: t := t * 1.0e+8;
			 4: t := t * 1.0e+16;
			 5: t := t * 1.0e+32;
			 6: t := t * 1.0e+64;
			 7: t := t * 1.0e+128;
			 8: t := t * 1.0e+256;
			end ;
			e := e div 2; i := i+1
		until e = 0;
		ten := t
	end { ten } ;

begin { at least 10 characters needed: b+9.9e+999 }		
	if undefined(x) then
	begin	repeat f^ := ' '; put(f); n := n-1
			until n <= 1;
		f^ := 'u'; put(f)
	end else
	if x = 0 then
	begin	repeat f^ := ' '; put(f); n := n-1
			until n <= 1;
		f^ := '0'; put(f)
	end else
	begin 
		if n < 10 then n := 3 else n := n-7;
		repeat f^ := ' '; put(f);  n := n-1
		until n <= 15;
		{ 1 < n < 15, number of digits to be printed }
		begin { test sign, then obtain exponent }
			if x < 0 then
				begin f^ := '-'; put(f); x := -x
				end else begin f^ := ' '; put(f) end;
			e := expo(x);
			if e >= 0 then
				begin e := e*77 div 256 + 1; x := x/ten(e);
					if x >= 1.0 then
						begin x := x/10.0; e := e+1
						end
					end else
					begin e := (e+1)*77 div 256; x := ten(-e)*x;
						if x < 0.1 then
							begin x := 10.0*x; e := e-1
							end
					end ;
			{ 0.1 <= x < 1.0 }
			case n of		{ rounding }
				 2: x := x+0.5e-2;
				 3: x := x+0.5e-3;
				 4: x := x+0.5e-4;
				 5: x := x+0.5e-5;
				 6: x := x+0.5e-6;
				 7: x := x+0.5e-7;
				 8: x := x+0.5e-8;
				 9: x := x+0.5e-9;
				10: x := x+0.5e-10;
				11: x := x+0.5e-11;
				12: x := x+0.5e-12;
				13: x := x+0.5e-13;
				14: x := x+0.5e-14;
				15: x := x+0.5e-15;
			end;
			if x >= 1.0 then
				begin x := x * 0.1; e := e+1;
				end ;
			c := trunc(x,48);
			c := 10*c; d := c div t48;
			f^ := chr(d+z); put(f);
			f^ := '.'; put(f);
			for i := 2 to n do
			begin c := (c - d*t48) * 10; d := c div t48;
				f^ := chr(d+z); put(f)
			end ;
			f^ := 'e'; put(f); e := e-1;
			if e < 0 then
				begin f^ := '-'; put(f); e := -e;
				end else begin f^ := '+'; put(f) end;
			e1 := e * 205 div 2048; e2 := e - 10*e1;
			e0 := e1 * 205 div 2048; e1 := e1 - 10*e0;
			f^ := chr(e0+z); put(f);
			f^ := chr(e1+z); put(f);
			f^ := chr(e2+z); put(f)
		end
	end
end {wre} ;

begin
	writeln('Hello, world!')
end.