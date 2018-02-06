
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

// added by me
#include <iostream>
#include <queue>
#include <map>

#include "board.h"
#include "minmax.h"
#include <stdlib.h>

cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size);

__global__ void addKernel(int *c, const int *a, const int *b)
{
    int i = threadIdx.x;
    c[i] = a[i] + b[i];
}

__global__ void c(int* c, int count) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    for (; i < count; i += blockDim.x * gridDim.x)
        c[i] = (1 << i) + 1;
}

int _main() {
	Board myBoard(3, 3, 3);
	int xpostion = 0;
	int ypostion = 0;

	while (1) {
		myBoard.Print();
		if (myBoard.IsTerminal()) {
			cout << "You something...";
			cin >> xpostion;
			break;
		}
		cout << endl << "Provide postion to set 'X' mark" << endl;
		
		//Validate position
		bool posIsTaken = true;
		//Check if pos is taken
		while (posIsTaken == true) {
			//init vars
			posIsTaken = false;
			xpostion = 0;
			ypostion = myBoard.GetRows() + 2;
			//Check if x is valid
			while (1) {
				cout << "x(1-" << myBoard.GetColumns() << "):"; cin >> xpostion;
				xpostion--;
				if (xpostion < 0 || xpostion >= myBoard.GetColumns()) {
					system("cmd /c cls");
					myBoard.Print();
					cout << endl << "Provide postion to set 'X' mark" << endl;
					cout << "Error: x value is not valid" << endl;
				}
				else {
					break;
				}
			}

			//Check if y is valid
			while (1) {
				cout << "y(1-" << myBoard.GetRows() << "):"; cin >> ypostion;
				ypostion--;
				if (ypostion < 0 || ypostion >= myBoard.GetRows()) {
					system("cmd /c cls");
					myBoard.Print();
					cout << endl << "Provide postion to set 'X' mark" << endl;
					cout << "Error: y value is not valid" << endl;
					cout << "x(1-" << myBoard.GetColumns() << "):" << xpostion << endl;
				}
				else {
					break;
				}
			}

			//Check pos is taken
			if (myBoard.GetCell(xpostion, ypostion) != 0) {
				posIsTaken = true;
				system("cmd /c cls");
				myBoard.Print();
				cout << endl << "Provide postion to set 'X' mark" << endl;
				cout << "Error: postion is taken" << endl;
			}
		}

		//Add pos and contiune
		myBoard.SetCell(xpostion, ypostion, PLAYER);
		//TO DO LOGIC
		//myBoard.SetCell(0, 0, OPPONENT);
		system("cmd /c cls");
	}

	return 0;
}

int main()
{
	return _main();
    int N = 2 << 13;
    Board _board(2, 2, 2);
    ppca::minmax mmAlg;

    GAME_CHAR player = PLAYER;

    int depth = _board.GetColumns() * _board.GetRows(),
        searchedDepth;

    int blockSize;      // The launch configurator returned block size 
    int minGridSize;    // The minimum grid size needed to achieve the maximum occupancy for a full device launch 
    int gridSize;       // The actual grid size needed, based on input size

    auto tasks = mmAlg.GetTasks(_board, player, N, depth, searchedDepth);
    map<unsigned int, long long int> results;

    while (!tasks.empty()) {
        Board& b = tasks.front();
        b.CalculateScore();
        results.insert(pair<unsigned int, long long int>(b.GetTreePosition(), rand() - (RAND_MAX / 2)));

        tasks.pop();
    }
    Board nextMove;
    mmAlg.GetBestMove(_board, player, results, depth);

    return 0;
    tasks = mmAlg.GetTasks(_board, player, N, depth, searchedDepth);


    float time;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, c, 0, tasks.size());

    // Round up according to array size 
    gridSize = (tasks.size() + blockSize - 1) / blockSize;

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Occupancy calculator elapsed time:  %3.3f ms \n", time);


    cudaEventRecord(start, 0);
    c << <gridSize, blockSize >> > (nullptr, 0);

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Kernel elapsed time:  %3.3f ms \n", time);

    auto cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "kernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        return 1;
    }

    return 0;

    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        return 1;
    }

    int size = 400000000;
    int* dev_c = 0;
    int* h_c = new int[size];
    memset(h_c, 0, sizeof(int)*size);

    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
    }

    cudaStatus = cudaMemcpy(dev_c, h_c, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
    }

    c << <14, 1024 >> > (dev_c, size);
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        return 0;
    }

    cudaStatus = cudaMemcpy(h_c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
    }

    cudaFree(dev_c);
    return 0;

    const int arraySize = 5;
    const int a[arraySize] = { 1, 2, 3, 4, 5 };
    const int b[arraySize] = { 10, 20, 30, 40, 50 };
    int c[arraySize] = { 0 };

    // Add vectors in parallel.
    cudaStatus = addWithCuda(c, a, b, arraySize);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addWithCuda failed!");
        return 1;
    }

    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",
        c[0], c[1], c[2], c[3], c[4]);

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size)
{
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel << <1, size >> > (dev_c, dev_a, dev_b);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }

    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);

    return cudaStatus;
}

