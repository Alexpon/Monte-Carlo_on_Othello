all: search.cpp
	g++ search.cpp -std=c++11 -O2 -o R05922068
judge: judge.cpp
	g++ judge.cpp -std=c++11 -O2 -o judge
test: R05922068 judge
	rm -rf log_ju.txt log_p1.txt log_p2.txt
	./judge 7122 > log_ju.txt &
	./R05922068 127.0.0.1 7122 > log_p1.txt &
	./R05922068 127.0.0.1 7122 > log_p2.txt &
