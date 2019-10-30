cpu_scheduling_output.txt: part2.o
    g++ part2.o -o cpu_scheduling_output.txt

part1.o: 'make part1.c'
    g++ -c 'make part1.c'

part2.o: 'make part2.c'
    g++ -c 'make part2.c'

clean:
    rm *.o cpu_scheduling_output.txt
