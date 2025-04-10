C_FLAGS = -g

all: main

./target/release/liblist_t.so:
	cargo build --release

main: main.c ./target/release/liblist_t.so
	clang $^ -o main ${C_FLAGS}

run: main
	./main

tester: tester.c ./target/release/liblist_t.so
	clang $^ -o test ${C_FLAGS}

test: tester
	./test

clean:
	cargo clean
	rm -f main
