# inf1029
Aulas de INF-1029 - Introdução a arquitetura de computadores

arquivo timer_test:
gcc -o timer_test timer_test.c timer.c
./timer_test <value>
exemplo prático:
./timer_test 1024000000

arquivo matrix_lib_test:
gcc -o matrix_lib_test matrix_lib_test.c matrix_lib.c timer.c
./matrix_lib_test <scalar> <heightA> <widthB> <heightB> <widthB> <floatsFile1> <floatsFile2> <resultFile1> <resultFile2>
exemplo prático:
./matrix_lib_test 2.0 3 3 3 3 "floats_256_2.0f.dat" "floats_256_5.0f.dat" "result1.dat" "result2.dat"

arquivo create_matrix:
gcc -o create_matrix create_matrix.c
./create_matrix 3 3 floats_256_2.0f.dat
./create_matrix 3 3 floats_256_5.0f.dat
