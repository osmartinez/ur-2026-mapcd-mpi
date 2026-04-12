APP_NAME=vector_sum_mpi
SRC=src/main.c
BIN=bin/$(APP_NAME)
NP?=4
N?=200000

.PHONY: build run run-mpi docker-build docker-run clean

build:
	@mkdir -p bin
	mpicc -O2 -Wall -Wextra -std=c11 $(SRC) -o $(BIN)

run: build
	$(BIN) $(N)

run-mpi: build
	mpirun --allow-run-as-root -np $(NP) $(BIN) $(N)

docker-build:
	docker build -t $(APP_NAME):latest .

docker-run:
	docker run --rm $(APP_NAME):latest mpirun --allow-run-as-root -np $(NP) /app/bin/$(APP_NAME) $(N)

clean:
	rm -rf bin
