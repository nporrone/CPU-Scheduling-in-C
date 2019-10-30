default: part1 part2

part1: part1.c
	gcc -pthread -o part1.out part1.c

part2: part2.c
	gcc -o part2.out part2.c

clean:
	rm *.out cpu_scheduling_output_file.txt
