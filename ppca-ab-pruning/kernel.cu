
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

// added by me
#include <iostream>
#include <queue>
#include <map>

#include "board.h"
#include "minmax.h"

__global__ void minmaxKernel(int taskCount, long long* results, int* data, unsigned int dataSize) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    /*for (; i < count; i += blockDim.x * gridDim.x)
        c[i] = (1 << i) + 1;*/
}

void FindBestMove(State& state, GAME_CHAR player, int depth) {
    auto cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaSetDevice failed! Do you have a CUDA-capable GPU installed?";
        exit(1);
    }

    int N = 2 << 13;
    ppca::minmax mmAlg;

    int searchedDepth;

    int blockSize;      // The launch configurator returned block size 
    int minGridSize;    // The minimum grid size needed to achieve the maximum occupancy for a full device launch 
    int gridSize;       // The actual grid size needed, based on input size

    auto tasks = mmAlg.GetTasks(state.GetBoard(), player, N, depth, searchedDepth);

    float time;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, minmaxKernel, 0, tasks.size());

    // Round up according to array size 
    gridSize = (tasks.size() + blockSize - 1) / blockSize;

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Occupancy calculator elapsed time:  %3.3f ms \n", time);

    int *dev_data, *host_data;
    long long *dev_results, *host_results;
    auto size = mmAlg.ConvertToGpuData(&host_data, tasks);

    cudaStatus = cudaMalloc((void**)&dev_data, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMalloc failed for data array!";
        exit(1);
    }

    host_results = new long long[tasks.size()];
    cudaStatus = cudaMalloc((void**)&dev_results, tasks.size() * sizeof(long long));
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMalloc failed for results array!";
        cudaFree(dev_data);
        exit(1);
    }

    cudaStatus = cudaMemcpy(dev_data, host_data, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMemcpy failed: data array to device!";
        cudaFree(dev_results);
        cudaFree(dev_data);
        exit(1);
    }


    cudaEventRecord(start, 0);
    minmaxKernel << <gridSize, blockSize >> > (tasks.size(), dev_results, dev_data, size);

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Kernel elapsed time:  %3.3f ms \n", time);

    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "kernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        cudaFree(dev_results);
        cudaFree(dev_data);
        exit(1);
    }

    cudaStatus = cudaMemcpy(host_results, dev_results, tasks.size() * sizeof(long long), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMemcpy failed: results array to host!";
        cudaFree(dev_results);
        cudaFree(dev_data);
        exit(1);
    }

    map<unsigned int, long long> results;
    auto it = tasks.begin();
    for (int i = 0; i < tasks.size(); i++, it++)
        results.insert(pair<unsigned int, long long>(it->GetTreePosition(), host_results[i]));

    State bestMove = mmAlg.GetBestMove(state.GetBoard(), player, results, searchedDepth);

    cudaFree(dev_results);
    cudaFree(dev_data);

    state = bestMove;
}

int game_loop() {
    Board _board(3, 3, 3);
    State state(PLAYER, _board);
    int depth = _board.GetRows() * _board.GetColumns();

    int xpostion = 0;
    int ypostion = 0;

    while (1) {
        Board* board = &state.GetBoard();
        board->Print();
        if (board->IsTerminal()) {
            if (state.GetScore() == 0)
                cout << "It was a DRAW.";
            else if (state.GetScore() < 0)
                cout << "You WON !!!";
            else
                cout << "You lost.";

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
            ypostion = board->GetRows() + 2;
            //Check if x is valid
            while (1) {
                cout << "x(1-" << board->GetColumns() << "):"; cin >> xpostion;
                xpostion--;
                if (xpostion < 0 || xpostion >= board->GetColumns()) {
                    system("cmd /c cls");
                    board->Print();
                    cout << endl << "Provide postion to set 'X' mark" << endl;
                    cout << "Error: x value is not valid" << endl;
                }
                else {
                    break;
                }
            }

            //Check if y is valid
            while (1) {
                cout << "y(1-" << board->GetRows() << "):"; cin >> ypostion;
                ypostion--;
                if (ypostion < 0 || ypostion >= board->GetRows()) {
                    system("cmd /c cls");
                    board->Print();
                    cout << endl << "Provide postion to set 'X' mark" << endl;
                    cout << "Error: y value is not valid" << endl;
                    cout << "x(1-" << board->GetColumns() << "):" << xpostion << endl;
                }
                else {
                    break;
                }
            }

            //Check pos is taken
            if (board->GetCell(xpostion, ypostion) != 0) {
                posIsTaken = true;
                system("cmd /c cls");
                board->Print();
                cout << endl << "Provide postion to set 'X' mark" << endl;
                cout << "Error: postion is taken" << endl;
            }
        }

        //Add pos and contiune
        board->SetCell(xpostion, ypostion, OPPONENT);
        depth--;

        if (!board->IsTerminal()) {
            FindBestMove(state, PLAYER, depth);
            depth--;
        }

        system("cmd /c cls");
    }

    return 0;
}

int main()
{
    game_loop();

    return 0;
}