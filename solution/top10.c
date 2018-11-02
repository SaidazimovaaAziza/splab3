#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
 // cat log.txt| cut -f1,10 -d" " | grep -v "-" |awk '{fruitCount[$1]+=$NF; next}END{for (fruit in fruitCount) printf "%s %s\n",fruit,fruitCount[fruit]}' | sort -n -r  -k2 | head -10 |awk -F':' '{ print $1,$2,$2*100}'
//cat log.txt| cut -f1,10 -d" " | grep -v "-" |awk '{sum[$1]+= $2;}END{for (date in sum){print date,sum[date];}}' | sort -n -r  -k2 | head -10 |awk '{ print $1,$2,$2*100/16055991,"%" }'
int main() {
	
	int A[2]; // pipe A
	pipe(A);
	pid_t pid1 = fork(); // cat
	
	if (!pid1) {
		dup2(A[1], 1);
		close(A[0]);
		close(A[1]);
		execlp("cat","cat", "log.txt", NULL);
		exit(EXIT_FAILURE);
	}
	
	int B[2]; // pipe B
	pipe(B);
	pid_t pid2 = fork();                                       
	if (!pid2) { // cut
		dup2(A[0], 0);
		close(A[0]);
		close(A[1]); 

		dup2(B[1], 1);
		close(B[0]);
		close(B[1]);      

		execlp("cut","cut", "-f1", "-f10","-d ", NULL);
		exit(EXIT_FAILURE);                            
	}
	
	// no need for pipe A anymore, parent closes it completely
	close(A[0]);
	close(A[1]);
	
	// pipe C is used to store result of $(cat log.txt | cut -f4 -d' ' | sed -E 's/\[0*([0-9])./\1/') to $list
	// but in our parent process there is no $list
	// we just read from the above command directly
	// and sum up all integers (see while loop below)
	int C[2]; 
	pipe(C);
	
	pid_t pid3 = fork();                                       
	if (!pid3) { // sed
		dup2(B[0], 0);
		close(B[0]);
		close(B[1]);
		
		dup2(C[1], 1);
		close(C[0]);
		close(C[1]);

		execlp("grep","grep","-v","\"-\"", NULL);
		exit(EXIT_FAILURE);    

	}
	
	// closing pipe ends that are no more used
	close(B[0]);
	close(B[1]);


	int D[2]; 
	pipe(D);
	pid_t pid4 = fork(); 

	if (!pid4) { // sed
		dup2(C[0], 0);
		close(C[0]);
		close(C[1]);
		
		dup2(D[1], 1);
		close(D[0]);
		close(D[1]);

		execlp("awk","awk", "{sum[$1]+= $2;}END{for (date in sum){print date,sum[date];}}", NULL);
		exit(EXIT_FAILURE);                            
	}
	
	close(C[0]);
	close(C[1]);

	int E[2]; 
	pipe(E);
	pid_t pid5 = fork(); 

	if (!pid5) { // sed
		dup2(D[0], 0);
		close(D[0]);
		close(D[1]);
		
		dup2(E[1], 1);
		close(E[0]);
		close(E[1]);

		execlp("sort","sort","-n","-r","-k2", NULL);
		exit(EXIT_FAILURE);                            
	}

	close(D[0]);
	close(D[1]);

	int F[2]; 
	pipe(F);

	pid_t pid6 = fork(); 

	if (!pid6) { // sed
		dup2(E[0], 0);
		close(E[0]);
		close(E[1]);
		
		dup2(F[1], 1);
		close(F[0]);
		close(F[1]);

		execlp("head","head","-10", NULL);
		exit(EXIT_FAILURE);                            
	}

	close(E[0]);
	close(E[1]);

	int G[2]; 
	pipe(G);

	pid_t pid7 = fork(); 

	if (!pid7) { // sed
		dup2(F[0], 0);
		close(F[0]);
		close(F[1]);
		
		dup2(G[1], 1);
		close(G[0]);
		close(G[1]);

		execlp("awk","awk","{sum+=$2}{ print $1,$2,$2/sum*100,\"%\"}", NULL);
		exit(EXIT_FAILURE);                            
	}

	close(F[0]);
	close(F[1]);

	// C[0] is used to calculate sum
	FILE* Cin = fdopen(G[0],"r"); // Cin is FILE* object made from C[0]
	int sum=0;
	char line[1000];
	while (fgets(line, sizeof(line), Cin)) {
		/* note that fgets don't strip the terminating \n, checking its
				presence would allow to handle lines longer that sizeof(line) */
		printf("%s", line); 
  }


	return 0;
}
