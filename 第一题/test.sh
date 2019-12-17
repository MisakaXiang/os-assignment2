#!/bin/bash -e
echo "Compiling"
gcc vm.c -o vm
echo "Running vm"
./vm -b BACKING_STORE.bin -f addresses.txt -p FIFO -n 256> out.txt
echo "Comparing with correct.txt"
diff out.txt correct.txt
