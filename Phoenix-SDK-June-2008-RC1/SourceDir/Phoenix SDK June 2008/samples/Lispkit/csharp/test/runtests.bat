REM Run tests by using the Lispkit interpreter.

Lispkit.exe -i -coverage inclist1.lispkit (1 2 4)
Lispkit.exe -i -coverage inclist2.lispkit (1 2 4)
Lispkit.exe -i -coverage mc91.lispkit 2
Lispkit.exe -i -coverage mc91.lispkit 100
Lispkit.exe -i -coverage compile.lispkit compile.lispkit

REM Run tests by using the Lispkit compiler.

Lispkit.exe quote-test.lispkit
quote-test.exe

Lispkit.exe arit-test.lispkit
arit-test.exe 4

Lispkit.exe fac.lispkit
fac.exe 9

Lispkit.exe fac-acc.lispkit
fac-acc.exe 9

Lispkit.exe if-test.lispkit
if-test.exe (4 4)
if-test.exe (4 3)
if-test.exe (A B)
if-test.exe (A A)
if-test.exe ((A 0) (A 0))
if-test.exe ((A) (B))

Lispkit.exe lambda-test-1.lispkit
lambda-test-1.exe (A B C)

Lispkit.exe lambda-test-2.lispkit
lambda-test-2.exe (A B C)

Lispkit.exe non-let-test.lispkit
non-let-test.exe (5 4)

Lispkit.exe let-test.lispkit
let-test.exe (5 4)

Lispkit.exe length.lispkit 
length.exe ((A (B) C) (1 2 4) 5 6 7 8)

Lispkit.exe inclist1.lispkit 
inclist1.exe (1 2 4)

Lispkit.exe inclist2.lispkit 
inclist2.exe (1 2 4)

Lispkit.exe append.lispkit 
append.exe ((1 2 3) (3 4 (5)))

Lispkit.exe mc91.lispkit
mc91.exe 2
mc91.exe 100

Lispkit.exe fib.lispkit
fib.exe (20 0 1)

Lispkit.exe compile.lispkit 
compile.exe compile.lispkit