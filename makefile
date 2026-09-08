myshell:myshell.cc
	g++ -o myshell myshell.cc -std=c++11
.PHONY:clean
clean:
	rm -f myshell