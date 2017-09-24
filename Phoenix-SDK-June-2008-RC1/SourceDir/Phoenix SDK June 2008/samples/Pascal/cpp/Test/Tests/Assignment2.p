{ Tests assignment statements. }

program Assignment2(output);

var empty_file1 : file of Integer;
	empty_file2 : file of Integer;
begin	
	empty_file1 := empty_file2; {error: cannot assign to a file variable}
end.