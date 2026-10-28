touch data.txt
chmod 600 data.txt
gcc -o prog main.c
./prog data.txt
chmod u+s prog