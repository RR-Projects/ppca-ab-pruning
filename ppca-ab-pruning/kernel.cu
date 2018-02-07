
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

// added by me
#include <iostream>
#include <queue>
#include <map>

#include "board.h"
#include "minmax.h"

__device__ class DevState;

__device__ struct DevPoint {
public:
    int x, y;

    __device__ DevPoint() { }

    __device__ DevPoint(int pos, int boardX) {
        x = pos % boardX;
        y = pos / boardX;
    }

    __device__ inline DevPoint operator-(const DevPoint& p) {
        DevPoint r;
        r.x = x - p.x;
        r.y = y - p.y;
        return r;
    }

    __device__ inline bool operator==(const DevPoint& p) {
        return x == p.x && y == p.y;
    }

    __device__ inline bool isZero() {
        return x == 0 && y == 0;
    }

    __device__ inline bool isAdjacent(const DevPoint& p) {
        return abs(x - p.x) < 2 && abs(y - p.y) < 2;
    }

};

__device__ class DevBoard {
private:
    unsigned int filledCells, nextMoveIterator;
    unsigned char columns, rows, lineLength, isTerminal;
    unsigned int* data;

    long long partialScore;

    __device__ DevBoard() : data(nullptr), filledCells(0), columns(0), rows(0), lineLength(0), isTerminal(0), partialScore(0) { }

public:
    friend class DevState;

    __device__ static void CreateInPlace(DevBoard* ptr, unsigned int* data) {
        ptr->nextMoveIterator = 0;
        ptr->filledCells = *data;
        data++;

        char* b = (char*)data;
        ptr->columns = *b;       b++;
        ptr->rows = *b;          b++;
        ptr->lineLength = *b;    b++;
        ptr->isTerminal = *b;    b++;
        data++;

        ptr->data = (unsigned int*)(ptr + 1);
        memcpy(ptr->data, data, ptr->GetDataStructureSize() * sizeof(unsigned int));
    }

    __device__ static void CreateInPlace(DevBoard* ptr, const DevBoard& b) {
        ptr->filledCells = b.filledCells;
        ptr->nextMoveIterator = 0;

        ptr->columns = b.columns;
        ptr->rows = b.rows;
        ptr->lineLength = b.lineLength;
        ptr->isTerminal = b.isTerminal;

        ptr->partialScore = b.partialScore;

        ptr->data = (unsigned int*)(ptr + 1);
        memcpy(ptr->data, b.data, ptr->GetDataStructureSize() * sizeof(unsigned int));
    }

    __device__ DevBoard(unsigned int* data) {
        CreateInPlace(this, data);
    }

    __device__ inline bool HasNextMove() const { return GetUpper16Bits(nextMoveIterator) < columns && GetLower16Bits(nextMoveIterator) < rows && (!IsTerminal()); }
    __device__ inline bool IsTerminal() const { return isTerminal || (columns * rows == filledCells); }

    __device__ inline unsigned int GetMaxDepth() const { return columns * rows - filledCells; }

    __device__ inline long long GetPartialScore() const { return partialScore; }

    __device__ inline unsigned int GetCell(const int& x, const int& y) const {
        return GetCellInternal(GetIndex(x, y), GetOffset(x, y));
    }

    __device__ void SetCell(int x, int y, unsigned int val) {
        int index = GetIndex(x, y);
        char offset = GetOffset(x, y);

        auto exVal = GetCellInternal(index, offset);

        auto exValBit = (((exVal + 1) >> 1) & 0x1);
        auto valBit = (((val + 1) >> 1) & 0x1);
        filledCells += (-1 * exValBit) | (valBit ^ exValBit);

        val = (val & CELL_BITMAP) << offset; // first two bits shifted by offset amount
        unsigned int clearPos = ~(CELL_BITMAP << offset);

        data[index] = (data[index] & clearPos) | val;

        CalculateScore();
    }

    __device__ void GetNextMove(DevBoard* move, GAME_CHAR player) {
        unsigned int
            i = GetUpper16Bits(nextMoveIterator),
            j = GetLower16Bits(nextMoveIterator);

        bool foundMove = false;

        for (; j < rows; j++) {
            for (; i < columns; i++) {

                if (GetCell(i, j) == 0) {

                    // found second move, update iterator and return
                    if (foundMove) {
                        nextMoveIterator = (i << 16) | (j & 0xFFFF);
                        return;
                    }
                    foundMove = true;

                    // found first move, set it to the return parameter
                    DevBoard::CreateInPlace(move, *this);
                    move->SetCell(i, j, player);
                }
            }
            i = 0;
        }

        nextMoveIterator = (columns << 16) | (rows & 0xFFFF);
    }

    __device__ inline unsigned int GetDataStructureSize() const {
        return ((this->columns * this->rows << 1) + 31) / (sizeof(int) << 3); // (x * y * 2 + 31) / (8 * sizeof(int)); 
    }

    __device__ void Print() {
        /*for (int i = 31; i >= 0; i--) {
            printf("%d", (data[0] >> i) % 2);
        }
        printf("\r\n");*/

        for (unsigned int j = 0; j < rows; j++) {
            for (unsigned int i = 0; i < columns; i++) {
                switch (GetCell(i, j))
                {
                case EMPTY:
                    printf("-");
                    break;
                case OPPONENT:
                    printf("X");
                    break;
                case PLAYER:
                    printf("0");
                    break;
                default:
                    break;
                }
            }
            printf("\r\n");
        }
    }


    __device__ void CalculateScore() {
        partialScore = rows * columns - filledCells; // reset

        DevPoint slowIncrement, fastIncrement;

        // horizontal move
        slowIncrement.x = 0;
        slowIncrement.y = 1;
        fastIncrement.x = 1;
        fastIncrement.y = 0;
        CalculateScoreOnDirection(slowIncrement, fastIncrement);

        // vertical move
        slowIncrement.x = 1;
        slowIncrement.y = 0;
        fastIncrement.x = 0;
        fastIncrement.y = 1;
        CalculateScoreOnDirection(slowIncrement, fastIncrement);

        // first diagonal, upper part
        slowIncrement.x = 1;
        slowIncrement.y = 0;
        fastIncrement.x = 1;
        fastIncrement.y = 1;
        CalculateScoreOnDirection(slowIncrement, fastIncrement, false);

        // first diagonal, lower part
        slowIncrement.x = 0;
        slowIncrement.y = 1;
        fastIncrement.x = 1;
        fastIncrement.y = 1;
        CalculateScoreOnDirection(slowIncrement, fastIncrement, false, true);

        // second diagonal, upper part
        slowIncrement.x = 1;
        slowIncrement.y = 0;
        fastIncrement.x = -1;
        fastIncrement.y = 1;
        CalculateScoreOnDirection(slowIncrement, fastIncrement);

        // second diagonal, lower part
        slowIncrement.x = 0;
        slowIncrement.y = 1;
        fastIncrement.x = -1;
        fastIncrement.y = 1;
        CalculateScoreOnDirection(slowIncrement, fastIncrement, true, true);
    }

private:
    __device__ void CalculateScoreOnDirection(DevPoint slowIncrement, DevPoint fastIncrement, bool startFromTopRight = false, bool applyInitialSlowIncrement = false) {
        unsigned int
            i = 0 + applyInitialSlowIncrement * slowIncrement.x + startFromTopRight * (columns - 1),
            j = 0 + applyInitialSlowIncrement * slowIncrement.y;

        for (; i >= 0 && j >= 0 && i < columns && j < rows; i += slowIncrement.x, j += slowIncrement.y) {


            unsigned int x = i, y = j;
            unsigned int counter = 0;
            GAME_CHAR lastSeen = EMPTY;
            for (; x >= 0 && y >= 0 && x < columns && y < rows; x += fastIncrement.x, y += fastIncrement.y) {
                auto val = GetCell(x, y);

                auto winOrLoss = 1 - 2 * CHAR_IS(val, OPPONENT);
                winOrLoss = winOrLoss * CHAR_NOT(CHAR_IS(val, EMPTY));

                counter = (counter * CHAR_IS(lastSeen, val)) + 1; // update counter or reset to 1

                partialScore += (winOrLoss * 4) << counter; // increment/decrement score by 2 << 1 up to 2 << lineLength
                partialScore += ((counter == lineLength) * winOrLoss) << 24; // handle win/loss (counter == lineLength) by adding or subtracting a large number (1 << 24)
                isTerminal |= (counter == lineLength) * CHAR_NOT(CHAR_IS(val, EMPTY));

                lastSeen = val;
            }
        }
    }

    __device__ inline unsigned int GetUpper16Bits(const unsigned int& val) const { return val >> 16; }
    __device__ inline unsigned int GetLower16Bits(const unsigned int& val) const { return val & 0xFFFF; }

    __device__ inline unsigned int GetCellInternal(const int& index, const char& offset) const {
        return (data[index] >> offset) & 0x3;
    }

    __device__ inline unsigned int GetIndex(const int& x, const int& y) const {
        return this->pos(x, y) / 32;
    }

    __device__ inline unsigned int GetOffset(const int& x, const int& y) const {
        return this->pos(x, y) % 32;
    }

    __device__ inline unsigned int pos(const int& x, const int& y) const {
        return (this->columns * y + x) << 1;
    }
};

__device__ class DevState {
private:
    long long score, alpha, beta;
    DevBoard board;

    __device__ DevState() : board() { }

public:

    __device__ static DevState* At(char* ptr, int offset, unsigned int boardDataSize) {
        unsigned long long size = CharSize(boardDataSize);
        return (DevState*)(ptr + size * offset);
    }

    __device__ static unsigned int CharSize(unsigned int boardDataSize) {
        unsigned long long sz = sizeof(DevState) + boardDataSize * sizeof(unsigned int);
        sz += sz % 8;
        return  sz;
    }

    __device__ static void CreateInPlace(DevState* ptr, GAME_CHAR player) {
        ptr->score = CHAR_IS(player, PLAYER) * INT64_MIN
            + CHAR_IS(player, OPPONENT) * INT64_MAX;

        ptr->alpha = INT64_MIN;
        ptr->beta = INT64_MAX;
    }

    __device__ static void CreateInPlace(DevState* ptr, GAME_CHAR player, const DevBoard& board) {
        CreateInPlace(ptr, player);
        DevBoard::CreateInPlace(&ptr->board, board);
    }

    __device__ inline void SetScore(const long long& score) { this->score = score; }
    __device__ inline long long GetScore() const { return score; }

    __device__ inline void SetAlpha(const long long& alpha) { this->alpha = alpha; }
    __device__ inline long long GetAlpha() { return alpha; }

    __device__ inline void SetBeta(const long long& beta) { this->beta = beta; }
    __device__ inline long long GetBeta() { return beta; }

    __device__ inline DevBoard& GetBoard() { return board; }

    __device__ inline DevBoard* GetBoardPtr() { return &board; }

    __device__ inline void SetBoard(const DevBoard& board) {
        DevBoard::CreateInPlace(&this->board, board);
    }
};

__device__ long long dev_minmax(const DevBoard &board, char* _stack, unsigned int dataStrSize, GAME_CHAR startPlayer, int depth) {
    int maxDepth = depth;

    int stackPos = 0;

    DevState::CreateInPlace(DevState::At(_stack, stackPos, dataStrSize), startPlayer, board);

    /*long long bestScore = CHAR_IS(startPlayer, PLAYER) * INT64_MIN
        + CHAR_IS(startPlayer, OPPONENT) * INT64_MAX;*/

    bool directyTerminal = true;

    while (stackPos >= 0) {
        DevState* parent = DevState::At(_stack, stackPos, dataStrSize);

        if (depth <= 0 || !parent->GetBoard().HasNextMove() || parent->GetBoard().IsTerminal()) {
            DevState* child = parent;
            stackPos--;

            if (stackPos < 0) { // reached top of tree
                if (directyTerminal)
                    child->SetScore(child->GetBoard().GetPartialScore());
                continue;
            }

            startPlayer = SWITCH_PLAYER(startPlayer);
            depth++;
            parent = DevState::At(_stack, stackPos, dataStrSize);

            auto score = child->GetBoard().GetPartialScore(); // getting the calculated score from the board

            auto prevScore = parent->GetScore();

            /*if (depth == maxDepth &&
                (CHAR_IS(startPlayer, PLAYER) && prevScore < score ||
                    CHAR_IS(startPlayer, OPPONENT) && prevScore > score))
                bestScore = score;*/

            parent->SetScore(
                CHAR_IS(startPlayer, PLAYER) * max(score, prevScore) + // MAX
                CHAR_IS(startPlayer, OPPONENT) * min(score, prevScore)  // MIN
            );

            parent->SetAlpha(
                CHAR_IS(startPlayer, PLAYER) * max(score, parent->GetAlpha()) +  // MAX
                CHAR_IS(startPlayer, OPPONENT) * parent->GetAlpha() // MIN
            );

            parent->SetBeta(
                CHAR_IS(startPlayer, PLAYER) * parent->GetBeta() + // MAX
                CHAR_IS(startPlayer, OPPONENT) * min(score, parent->GetBeta())  // MIN
            );

            if (parent->GetAlpha() > parent->GetBeta()) { // alpha beta pruning

                stackPos--;
                startPlayer = SWITCH_PLAYER(startPlayer);
                depth++;
            }

            directyTerminal = false;
        }
        else if (parent->GetBoard().HasNextMove()) {
            unsigned int tPos;
            DevState* movePtr = DevState::At(_stack, stackPos + 1, dataStrSize);
            DevState::CreateInPlace(movePtr, SWITCH_PLAYER(startPlayer));

            parent->GetBoard().GetNextMove(movePtr->GetBoardPtr(), startPlayer);

            movePtr->SetAlpha(parent->GetAlpha());
            movePtr->SetBeta(parent->GetBeta());

            stackPos++;
            depth--;
            startPlayer = SWITCH_PLAYER(startPlayer);
        }
    }

    DevState* root = DevState::At(_stack, 0, dataStrSize);
    return root->GetScore();
}

/// --------------- KERNEL
__global__ void minmaxKernel(int taskCount, void* dev_stack, long long* results, unsigned int* data, unsigned int dataItemSize, unsigned int dataStructureSize, unsigned int maxDepth) {
    int threadPos = blockIdx.x * blockDim.x + threadIdx.x;

    for (int i = threadPos; i < taskCount; i += blockDim.x * gridDim.x) {

        DevState* stack = DevState::At((char*)dev_stack, threadPos * (maxDepth + 1), dataStructureSize);
        DevBoard b(data + i * dataItemSize);

        if (maxDepth == 0) {
            b.CalculateScore();
            results[i] = b.GetPartialScore();
        }
        else {
            long long bestScore = dev_minmax(b, (char*)stack, dataStructureSize, PLAYER, maxDepth);
            results[i] = bestScore;
        }
    }
}

void FindBestMove(State& state, GAME_CHAR player, int depth) {
    auto cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaSetDevice failed! Do you have a CUDA-capable GPU installed?";
        exit(1);
    }

    cudaDeviceSetLimit(cudaLimitMallocHeapSize, 16 * 1024 * 1024);
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaDeviceSetLimit failed!";
        exit(1);
    }

    int N = 2 << 8;
    ppca::minmax mmAlg;

    int searchedDepth;

    int blockSize;      // The launch configurator returned block size 
    int minGridSize;    // The minimum grid size needed to achieve the maximum occupancy for a full device launch 
    int gridSize;       // The actual grid size needed, based on input size

    auto tasks = mmAlg.GetTasks(state.GetBoard(), player, N, depth, searchedDepth);
    int gpuDepth = min(4, depth - searchedDepth);

    cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, minmaxKernel, 0, tasks.size());

    // Round up according to array size 
    gridSize = (tasks.size() + blockSize - 1) / blockSize;
    //blockSize = 1;

    cudaDeviceSynchronize();

    void* dev_stack;
    unsigned int *dev_data, *host_data;
    long long *dev_results, *host_results;
    auto size = mmAlg.ConvertToGpuData(&host_data, tasks);
    auto dataElemCount = tasks.front().GetDataStructureSize();

    cudaStatus = cudaMalloc((void**)&dev_data, size * sizeof(unsigned int));
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

    unsigned long long dSize = dataElemCount * sizeof(unsigned int) + sizeof(DevState);
    dSize += dSize % 8;
    cudaStatus = cudaMalloc((void**)&dev_stack, (gpuDepth + 1) * gridSize * blockSize * dSize);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMalloc failed for results array!";
        cudaFree(dev_data);
        exit(1);
    }

    cudaStatus = cudaMemcpy(dev_data, host_data, size * sizeof(unsigned int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMemcpy failed: data array to device!";
        cudaFree(dev_results);
        cudaFree(dev_data);
        exit(1);
    }

    minmaxKernel << <gridSize, blockSize >> > (tasks.size(), dev_stack, dev_results, dev_data, size / tasks.size(), dataElemCount, gpuDepth);

    cudaDeviceSynchronize();

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
    Board _board(4, 4, 3);
    State state(PLAYER, _board);
    int depth = _board.GetRows() * _board.GetColumns();

    int xpostion = 0;
    int ypostion = 0;

    while (1) {
        Board* board = &state.GetBoard();
        board->Print();
        if (board->IsTerminal()) {
            auto winner = board->GetWinner();
            if (winner == EMPTY)
                cout << "It was a DRAW.";
            else if (winner == OPPONENT)
                cout << "You WON !!!";
            else if (winner == PLAYER)
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