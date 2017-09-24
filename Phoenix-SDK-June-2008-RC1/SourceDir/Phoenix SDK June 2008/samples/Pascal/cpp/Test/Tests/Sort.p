(*****************************************************************************
 * A simple bubble sort program.  Reads integers, one per line, and prints   *
 * them out in sorted order.  Blows up if there are more than 49.            *
 *****************************************************************************)
PROGRAM Sort(input, output, bubblesort);
    CONST
	(* Max array size. *)
	MaxElts = 50;
    TYPE 
	(* Type of the element array. *)
	IntArrType = ARRAY [1..MaxElts] OF Integer;

    VAR
	(* Indexes, exchange temp, array size. *)
	i, j, tmp, size: integer;

	(* Array of ints *)
	arr: IntArrType;

	(* File data *)
	bubblesort : file of integer;
	
    BEGIN
	(* Read *)
	size := 1;
	WHILE NOT eof(bubblesort) DO BEGIN
		readln(bubblesort, arr[size]);
	IF NOT eof(bubblesort) THEN 
	    size := size + 1
	END;

	(* Sort using bubble sort. *)
	FOR i := size - 1 DOWNTO 1 DO
	    FOR j := 1 TO i DO 
		IF arr[j] > arr[j + 1] THEN BEGIN
		    tmp := arr[j];
		    arr[j] := arr[j + 1];
		    arr[j + 1] := tmp;
		END;

	(* Print. *)
	FOR i := 1 TO size DO
	    writeln(arr[i])
    END.
	    