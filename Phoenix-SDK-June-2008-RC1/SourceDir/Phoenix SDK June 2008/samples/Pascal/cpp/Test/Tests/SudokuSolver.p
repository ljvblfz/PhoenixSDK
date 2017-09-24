{
   A simple-minded brute-force sudoku puzzle solver.

   The input is a file of 81 numbers, 0-9 (0 representing
   blank squares). Example:
      1 4 9 6 0 0 7 0 2
      0 0 0 3 0 7 4 8 0
      0 8 0 0 1 0 9 0 0
      0 0 4 0 2 9 0 5 0
      0 1 0 8 4 3 0 7 0
      0 7 0 5 6 0 3 0 0
      0 0 6 0 3 0 0 4 0
      0 5 1 9 0 6 0 0 0
      8 0 3 0 0 4 6 1 7
   The program expects a valid sudoku. No validation is performed.

   The program performs an exhaustive search and should find
   multiple solutions if they exist.
}

program SudokuSolver(sudoku, output);
   type
      index9 = 0..8;  { Suitable for rows, columns, and gridlet numbers. }
   var
      A : array[ index9, index9] of integer; { This holds the puzzle.
                                               There's only one A; all work
                                               is done in-place on A. }

   procedure ReadA;
      var
         sudoku : file of integer;
         i, j : index9;
   begin
      for i := 0 to 8 do
         for j := 0 to 8 do
            read(sudoku, A[i,j])
   end;

   procedure PrintA;
      var
         i, j : index9;
   begin
      for i := 0 to 8 do
      begin
         for j := 0 to 8 do
            if A[i,j] <> 0 then
               write( A[i,j], ' ' )
            else
               write( '- ' );
         writeln
      end
   end;

   function Legal( i, j : index9 ) : boolean;
   {
      Tests the legality of the row, column, and gridlet
      containing A[i,j].
   }

      function LegalRow( i : index9 ) : boolean;
      {
         Tests the legality of the row containing A[i,j].
      }
         var
            exists : array [ 1..9 ] of boolean;
            k : index9;
            n : integer;
            retval : boolean;
      begin
         for n := 1 to 9 do
            exists[ n ] := false;
         retval := true;
         for k := 0 to 8 do
            if A[ i, k ] <> 0 then
            begin
               retval := retval and not exists[ A[ i, k ] ];
               exists[ A[ i, k ] ] := true;
            end;
         LegalRow := retval
      end;

      function LegalCol( j : index9 ) : boolean;
      {
         Tests the legality of the column containing A[i,j].
      }
         var
            exists : array [ 1..9 ] of boolean;
            k : index9;
            n : integer;
            retval : boolean;
      begin
         for n := 1 to 9 do
            exists[ n ] := false;
         retval := true;
         for k := 0 to 8 do
            if A[ k, j ] <> 0 then
            begin
               retval := retval and not exists[ A[ k, j ] ];
               exists[ A[ k, j ] ] := true;
            end;
         LegalCol := retval
      end;

      function Legal3x3( i, j : index9 ) : boolean;
      {
         Tests the legality of the 3x3 gridlet containing A[i,j].
      }
         var
            exists : array [ 1..9 ] of boolean;
            k : index9;
            n : integer;
            retval : boolean;

         {
            The nine squares of the gridlet are numbered 0..8.
            The following functions convert k, the index of a
            gridlet square, into the row and column coordinates
            of that square. i and j must be set to the upper
            left corner of the gridlet *before* calling these fns.
         }
         function R( k : index9 ) : integer;
         begin
            R := k div 3 + i
         end;

         function C( k : index9 ) : integer;
         begin
            C := k mod 3 + j
         end;

      begin
         for n := 1 to 9 do
            exists[ n ] := false;
         retval := true;
         i := ( i div 3 ) * 3;  { point i and j to the upper }
         j := ( j div 3 ) * 3;  {  left corner of the gridlet. }
         for k := 0 to 8 do
            if A[ R(k), C(k) ] <> 0 then
            begin
               retval := retval and not exists[ A[ R(k), C(k) ] ];
               exists[ A[ R(k), C(k) ] ] := true;
            end;
         Legal3x3 := retval
      end;

   begin
      Legal := LegalRow( i ) and LegalCol( j ) and Legal3x3( i, j );
   end;

   procedure Solve( p : integer );
   {
      A value of p in [0..80] indicates one of the 81 puzzle squares.
      This procedure recursively looks at each puzzle square. If the
      contents are non-zero, then it's a fixed value set by the puzzle
      and Solve moves on to the next square.
      Otherwise it's a blank square, so Solve tries out all possible
      values for the square, eliminating any that result in illegal
      board configurations. When Solve finds a legal candidate, it
      calls itself to examine the next square.

      If p reaches 81, that means we've gone through the board and
      have found a solution.

      Note that Solve keeps going after it's found a solution, so
      it'll find multiple solutions if they exist.
   }
      var
         i, j : index9;
         try : integer;

      function ConvertToRow( p : integer ) : index9;
      begin
         ConvertToRow := p div 9
      end;

      function ConvertToCol( p : integer ) : index9;
      begin
         ConvertToCol := p mod 9
      end;

   begin
      if p < 81 then
      begin
         i := ConvertToRow( p );
         j := ConvertToCol( p );
         if A[ i, j ] <> 0 then  { this value is fixed }
            Solve( p + 1 )
         else            { this is a blank square... }
         begin
            for try := 1 to 9 do
            begin
               A[ i, j ] := try;
               if Legal( i, j ) then
                  Solve( p + 1 )
            end;
            A[ i, j ] := 0  { revert to original state }
         end
      end
      else
      begin
         writeln( 'Solution:' );
         PrintA
      end
   end;

begin
   ReadA;
   writeln( 'Puzzle:' );
   PrintA;
   Solve( 0 )
end.