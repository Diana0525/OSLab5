cc = gcc
prom = shell
deps = disk.h
obj = file.o disk.o msh.o

$(prom): $(obj)
	$(cc) -o $(prom) $(obj)

%.o: %.c $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)
