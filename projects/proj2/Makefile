NAME=proj2

proj2:
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -g $(NAME).c -o $(NAME) -lrt -lpthread

run:
	./$(NAME) 2 100 100 100 100 5
	cat $(NAME).out
run1:
	./$(NAME) 6 100 100 100 100 5
	cat $(NAME).out
run2:
	./$(NAME) 100 100 100 100 100 10
	cat $(NAME).out
clean:
	rm ~/../../dev/shm/*xyadlo00*
