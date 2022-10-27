#include "asg2.h"
#include <stdio.h>
#include <mpi.h>


int rank;
int world_size;


void master(int world_size) {
	//TODO: procedure run in master process
	// MPI_Scatter...
	// MPI_Gather...
	// the following code is not a necessary, please replace it with MPI implementation.
	for (int ini_row = 0; ini_row < world_size - 1; ini_row++) {
		MPI_Send(&ini_row, 1, MPI_INT, ini_row + 1, ini_row + 1, MPI_COMM_WORLD);
	}

	float colors[Y_RESN];
	for (int row = world_size - 1; row < X_RESN; row++) {
		MPI_Status stat;
		MPI_Recv(colors, Y_RESN, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
		MPI_Send(&row, 1, MPI_INT, stat.MPI_SOURCE, stat.MPI_SOURCE, MPI_COMM_WORLD);
		// put the result to data storage
		int target_row = stat.MPI_TAG;
		Point* p = data + target_row * Y_RESN;
		// printf("%d, %d \n", target_row, p->x);
		for (int col = 0; col < Y_RESN; col++) {
			p->color = colors[col];
			p = p + 1;
		}
		// printf("%d, %d \n", target_row, p->x);
	}
	int termi_flag = -1;
	for (int i = 1; i < world_size; i++) {
		MPI_Status stat;
		MPI_Recv(colors, Y_RESN, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
		MPI_Send(&termi_flag, 1, MPI_INT, stat.MPI_SOURCE, stat.MPI_SOURCE, MPI_COMM_WORLD);
		// put the result to data storage
		int target_row = stat.MPI_TAG;
		Point* p = data + target_row * Y_RESN;
		for (int col = 0; col < Y_RESN; col++) {
			p->color = colors[col];
			p = p + 1;
		}
	}
	
	//TODO END

}


void slave(int world_size, int rank) {
	//TODO: procedure run in slave process
	// MPI_Scatter...
	// MPI_Gather...
	// int work[4];

	while (true) {
		// send a flag to tell the master that this slave process is ready for next work
		int row;
		MPI_Recv(&row, 1, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (row == -1) break;
		
		Point* p = new Point;
		p->x = row;
		p->y = 0;
		p->color = 0.0;
		float colors[Y_RESN];
		for (int col = 0; col < Y_RESN; col++) {
			compute(p);
			colors[col] = p->color;
			p->y = p->y + 1;
		}
		
		MPI_Send(colors, Y_RESN, MPI_FLOAT, 0, row, MPI_COMM_WORLD);
	}
	
	//TODO END
}


int main(int argc, char *argv[]) {
	if ( argc == 4 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
	}

	/* computation part begin */
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (rank == 0) {
		#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(500, 500); 
		glutInitWindowPosition(0, 0);
		glutCreateWindow("MPI");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, X_RESN, 0, Y_RESN);
		glutDisplayFunc(plot);
		#endif
	}

	if (rank == 0) {
		t1 = std::chrono::high_resolution_clock::now();

		initData();
		master(world_size);
		// Point* p = data;
		// printf("%d, %d\n", p->x, p->y);
		// p = p + 2 * Y_RESN;
		// printf("%d, %d\n", p->x, p->y);

		t2 = std::chrono::high_resolution_clock::now();  
		time_span = t2 - t1;

		printf("Student ID: 119010369\n"); // replace it with your student id
		printf("Name: Bodong Yan\n"); // replace it with your name
		printf("Assignment 2 MPI\n");
		printf("Run Time: %f seconds\n", time_span.count());
		printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
		printf("Process Number: %d\n", world_size);
		
	} else {
		slave(world_size, rank);
	}

	MPI_Finalize();
	/* computation part end */

	if (rank == 0){
		#ifdef GUI
		glutMainLoop();
		#endif
	}

	return 0;
}

