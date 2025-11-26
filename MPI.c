#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

int main(int argc , char** argv){

    int choice,rc,rank,numtasks,n,elemperproc;
    double *X=NULL;//vector of elements
    double *local_X=NULL;//local vector for each process

    MPI_Status status;
    rc = MPI_Init(&argc, &argv); //initialize MPI environment

    if (rc != 0) {
    printf ("Error in initialising MPI\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
    }




    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

do{
//0 process interacts with user
if (rank == 0) {
    printf("---------MENU---------\n");
    printf("1.Continue\n");
    printf("2.Exit\n");
    printf("----------------------\n");
    printf("Enter your choice : \n");

    scanf("%d",&choice);
    

    for (int dest = 1; dest < numtasks; dest++) {
            MPI_Send(&choice, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    } else {
            // Workers wait to receive the command
            MPI_Recv(&choice, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

   

    if (choice == 2){
        if(rank == 0){
            printf("Exiting program...\n");
            break;
        }
    }
    
    if (choice != 1){
        if (rank==0){
        printf("Invalid choice, please try again.\n");
        }
        continue;
    }

    if(choice == 1){
        
        if (rank == 0){

            printf("Continuing...\n");

            //giving elements from user
            printf("Enter the number of n elements :\n");

            do{
            scanf("%d",&n);
            }while(n<=0);//want a positive number


            //initialize dynamically the array
            X=(double*)malloc(n*sizeof(double));

            printf("Enter %d elements :\n",n);
            for(int i=0;i<n;i++){
                scanf("%lf",&X[i]);
            }
        }

            if (rank == 0) { //first process sends n to all other processes
            for (int dest = 1; dest < numtasks; dest++) {
                MPI_Send(&n, 1, MPI_INT, dest, 10, MPI_COMM_WORLD);
            }
             } else {
                MPI_Recv(&n, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }


            //handling n%p != 0
            //process 0 will take the extra elements if n%p != 0

            int elemperproc = n/numtasks; //number of elements per process
            int remainder = n % numtasks; //remaining elements
            int local_n ;

            
            if (rank == 0) {
                local_n = elemperproc + remainder; //process 0 takes the extra elements
             }else{
                local_n = elemperproc;
             } 
             
             local_X =(double*)malloc((local_n*sizeof(double)));// allocate local array for each process
             
             if(rank == 0){
                for (int i =0;i<local_n;i++){
                    local_X[i] = X[i];
                }
             

             int offset = local_n; //starting point for each process for example if proc 0 takes [0,1,2] (3 elements) then rank 1 will start from 3 till 3+elemperproc-1
             for (int dest =1 ;dest < numtasks; dest++){
                MPI_Send(&X[offset], elemperproc, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
                offset += elemperproc;
             } 
        }else {
                MPI_Recv(local_X,local_n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
             }

             //(a) avg calculation
             double local_sum =0.0;
             for (int i=0;i<local_n;i++){
                local_sum += local_X[i]; // each process calculates its local sum
             }

             double total_sum=0.0;
             double average;
             if (rank ==0){ 
                total_sum = local_sum;
                double temp_sum;
                for (int source =1; source < numtasks; source++){
                    MPI_Recv(&temp_sum, 1, MPI_DOUBLE, source, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//receive local sums from other processes
                    total_sum += temp_sum;
                }
                average = total_sum / n;
                printf("\nTotal Sum = %lf\n", total_sum);
                printf("Average = %lf\n", average);
                for (int dest = 1; dest < numtasks; dest++) {
                    MPI_Send(&average, 1, MPI_DOUBLE, dest, 25, MPI_COMM_WORLD);// send average to other processes
                }
             } else {
               MPI_Send(&local_sum, 1, MPI_DOUBLE, 0, 20, MPI_COMM_WORLD);
                MPI_Recv(&average, 1, MPI_DOUBLE, 0, 25, MPI_COMM_WORLD, &status);
             }

             
             

             //(b)max calculation , each process calculates its local max
             double local_max = local_X[0];//initialize local max with first element
             for (int i=1;i<local_n;i++){
                if (local_X[i] > local_max){
                    local_max = local_X[i];
                }
            }
            //same procudeure as avg to find global max
            double global_max;
            if(rank==0){
                global_max = local_max;
                double temp_max;
                for (int source =1; source < numtasks; source++){
                    MPI_Recv(&temp_max, 1, MPI_DOUBLE, source, 30, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if (temp_max > global_max){
                        global_max = temp_max;
                    }
                }
                printf("Global Max = %lf\n", global_max);
                for (int dest = 1; dest < numtasks; dest++) {
                MPI_Send(&global_max, 1, MPI_DOUBLE, dest, 35, MPI_COMM_WORLD);
                }
            } else {
                MPI_Send(&local_max, 1, MPI_DOUBLE, 0, 30, MPI_COMM_WORLD);
                 MPI_Recv(&global_max, 1, MPI_DOUBLE, 0, 35, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            //(c)
            double local_var = 0.0;
            for (int i=0;i<local_n;i++){
                local_var += pow((local_X[i] - average) ,2);
            }

            if (rank ==0){
                double total_var = local_var;
                double temp_var;
                for (int source =1; source < numtasks; source++){
                    MPI_Recv(&temp_var, 1, MPI_DOUBLE, source, 40, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    total_var += temp_var;
                }
                double variance = total_var / n;
                printf("Variance = %lf\n", variance);
            } else {
                MPI_Send(&local_var, 1, MPI_DOUBLE, 0, 40, MPI_COMM_WORLD);
            }

            //(d)
            double *local_delta = (double*)malloc(local_n*sizeof(double));
            for (int i=0;i<local_n;i++){
                local_delta[i] = pow((local_X[i] - global_max),2);//each process calculates its local delta values
            }

            double *delta = NULL;
            if (rank ==0){
                delta = (double*)malloc(n*sizeof(double));


                for (int i=0;i<local_n;i++){
                    delta[i] = pow((local_X[i] - global_max),2) ;
                }

                int offset = local_n;
                for (int source =1; source < numtasks; source++){
                   MPI_Recv(&delta[offset], elemperproc, MPI_DOUBLE, source, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    offset += elemperproc;
                }

                printf("\nDelta values:\n");
                for (int i=0; i<n; i++){
                    printf("Delta[%d] = %lf\n", i, delta[i]);
                }
            }else {
                    MPI_Send(local_delta, local_n, MPI_DOUBLE, 0, 4, MPI_COMM_WORLD);
            }

                //free allocated memory
                if (rank ==0){
                free(X);
                free(delta);
                }
                free(local_X);
                free(local_delta);
                

        }

    }while (choice != 2);

MPI_Finalize(); //finalize MPI environment
return 0;


}
