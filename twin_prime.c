#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define YES 1
#define NO 0

/* Prime checking method */

int is_prime(int n) {
  int i;
  if (n == 1) {
    return NO;
  }
  for (i = 2; i*i <= n; i++) {
    if (n % i == 0) {
      return NO;
    }
  }
  return YES;
}

/* Twin prime finder:
   In: num_tprimes = number of twin primes to find,
       odd_even = 0, 1, or 2 (multiplier for the 30),
       last_num = 1, 7, or 9 (the last digit of the prime to check,
       tprimes = array to store twin primes found.
   Logic for only searching through the numbers that have the possibility
   of being twin primes is based on the conjecture:
     Twin primes (p, p + 2) only occur if p is of the form:
       11 + 30n
       17 + 30n
       29 + 30n
*/

void tprime_finder(int num_tprimes, int odd_even, int last_num, int* tprimes) {
  int i, x = 0, t = 0;

  if (last_num == 1) {
    for (i = 0; t < num_tprimes; i++) {
      x = (11 + (30 * odd_even)) + (90 * i);
      if (is_prime(x)) {
        if (is_prime(x + 2)) {
          tprimes[t] = x;
          t++;
        }
      }
    }
  }
  else if (last_num == 7) {
    for (i = 0; t < num_tprimes; i++) {
      x = (17 + (30 * odd_even)) + (90 * i);
      if (is_prime(x)) {
        if (is_prime(x + 2)) {
          tprimes[t] = x;
          t++;
        }
      }
    }
  }
  else if (last_num == 9) {
    for (i = 0; t < num_tprimes; i++) {
      x = (29 + (30 * odd_even)) + (90 * i);
      if (is_prime(x)) {
        if (is_prime(x + 2)) {
          tprimes[t] = x;
          t++;
        }
      }
    }
  }
}

/* Comparator for quick sort */

int compar(const void * p1, const void * p2) {
  return ( *(int*)p1 - *(int*)p2 );
}

////////////////////////////////////////////////////////
/*                       Main                         */
////////////////////////////////////////////////////////

main(int argc, char** argv) {
  //Usage error
  if (argc != 2) {
    fprintf(stderr, "Usage: twin_prime num_primes_to_search (- 2)\n");
    exit(1);
  }

  //Variable declarations
  int num_tprimes, num_tprimes_per_proc, num_leftover, odd_even, last_num, i;
  int* local_tprimes;
  int* total_tprimes;
  int my_rank, comm_sz, name_length;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  //MPI Initialization
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Get_processor_name(processor_name, &name_length);

  //Number of primes definitions
  num_tprimes = atoi(argv[1]) - 2;
  num_tprimes_per_proc = (num_tprimes / comm_sz);
  num_leftover = (num_tprimes % comm_sz);
  //Leftover statement in case num_primes_to_search is not divisible by 9
  if (my_rank == (comm_sz - 1))
    num_tprimes_per_proc += num_leftover;

  //Allocate memory for the arrays
  local_tprimes = (int *)malloc(num_tprimes_per_proc * sizeof(int));
  total_tprimes = (int *)malloc(num_tprimes * sizeof(int));

  //Row assignments for processes
  if (my_rank == 0 || my_rank == 1 || my_rank == 2) {
    odd_even = 0;
  }
  else if (my_rank == 3 || my_rank == 4 || my_rank == 5) {
    odd_even = 1;
  }
  else if (my_rank == 6 || my_rank == 7 || my_rank == 8) {
    odd_even = 2;
  }

  //Col assignments for processes
  if (my_rank == 0 || my_rank == 3 || my_rank == 6) {
    last_num = 1;
  }
  else if (my_rank == 1 || my_rank == 4 || my_rank == 7) {
    last_num = 7;
  }
  else if (my_rank == 2 || my_rank == 5 || my_rank == 8) {
    last_num = 9;
  }

  //Each process finds however many twin prime pairs it needs to
  tprime_finder(num_tprimes_per_proc, odd_even, last_num, local_tprimes);
  fprintf(stderr, "Process number %d at %s finished finding twin primes\n",
          my_rank, processor_name);

  //Wait for everyone then gather the list together
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Gather(local_tprimes, num_tprimes_per_proc, MPI_INT, total_tprimes,
             num_tprimes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  //Diagnostic printing and final output
  if (my_rank == 0) {
    fprintf(stderr, "Gathered lists now sorting...\n");
    qsort(total_tprimes, num_tprimes, sizeof(int), compar);
    fprintf(stderr, "Sorted lists now printing.\n");
    printf("Found %d twin prime pairs:\n", num_tprimes);
    printf("(3, 5) (5, 7) ");
    for (i = 0; i < num_tprimes; i++) {
      printf("(%d, %d) ", total_tprimes[i], (total_tprimes[i] + 2));
    }
    printf("\n");
    fprintf(stderr, "Done.\n");
  }

  MPI_Finalize();
}
