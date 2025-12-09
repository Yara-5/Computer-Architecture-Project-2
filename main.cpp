
#include "ROB.cpp"


// Global Constants and Types

const int NUM_REGS = 8;
const int MEMORY_SIZE = 65536;  // 128 KB word-addressable

// Station Constants

int reserve_num[] = {2, 1, 2, 1, 4, 2, 1};
int reserve_start[7];
int cycles_num[] = { 2,2,1,1,2,1,12 };
int ReadMemoryTime = 4;
int WriteMemoryTime = 4;
int TotalReserveStations;
int ROBSize = 8;

struct Instruction {
    char opcode;
    int dst, src1, src2;
    int16_t imm;       // offset or immediate
    int pc;            // program counter
};


// Reservation Station entry
struct RSEntry {
    bool busy;
    char op;
    int16_t Vj, Vk;
    int Qj, Qk;
    int robIndex;
    int executionCyclesLeft;
    int16_t address;   // for load/store

    RSEntry(bool b, char o, int16_t vj, int16_t vk, int qj, int qk, int rind, int excl, int16_t add) : busy(b),
        op(o), Vj(vj), Vk(vk), Qj(qj), Qk(qk), robIndex(rind), executionCyclesLeft(excl), address(add) {}
};


// Global Simulator State

vector<Instruction> programMemory;         // Instructions
int16_t dataMemory[MEMORY_SIZE];           // Data memory
int16_t registers[NUM_REGS];               // Register file
int regStatus[NUM_REGS];                   // ROB index producing reg (-1 if free)

ROB rob(ROBSize);                                 // Initializing Reorder buffer
vector<RSEntry> reservationStations;        // All RS entries

vector<vector<int>> stores(ROBSize);        // 3 values: address, ready?, value         It's used to signify which datamemory items are about to be written to


int pc = 0;                                 // Program counter
int cycle = 0;                              // Global cycle counter


// Phase 1: Initialization

void loadProgram();                           // Load instructions into programMemory

void initRegisters() {                        // Initialize registers and regStatus
    registers[0] = 0;
    for (auto st : regStatus)
        st = -1;
}

void initMemory() {                           // Initialize dataMemory
    cout << "Do you need to enter data into the data memory?\nIf so press 1, else press 0.\n";
    int ans;
    int16_t address, data;
    cin >> ans;
    while (ans) {
        cout << "Enter the address you want to enter data to: ";
        cin >> address;
        cout << "Enter the data you want to enter at address " << address << ": ";
        cin >> data;
        dataMemory[address] = data;
        cout << "\nDo you want to enter more data?\n If yes, press 1, else press 0\n";
    }
    cout << "\nThe program will start running now.\n";
    for (auto s : stores)
        s = { -1,0,0 };
}

void initReservationStations() {              // Initialize all RS entries
    for (auto rs : reservationStations)
        rs.busy = false;
}

// Phase 2: Issue

bool canIssue(const Instruction& inst, int& i) {
    if (rob.isFull())
        return false;
    switch (inst.opcode) {
    case 'l':
        i = reserve_start[0];
        while (reserve_start[1] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 't':
        i = reserve_start[1];
        while (reserve_start[2] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'b':
        i = reserve_start[2];
        while (reserve_start[3] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'c':
        i = reserve_start[3];
        while (reserve_start[4] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'r':
        i = reserve_start[3];
        while (reserve_start[4] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'a':
        i = reserve_start[4];
        while (reserve_start[5] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 's':
        i = reserve_start[4];
        while (reserve_start[5] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'n':
        i = reserve_start[5];
        while (reserve_start[6] > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    case 'm':
        i = reserve_start[6];
        while (TotalReserveStations > i) {
            if (!reservationStations[i].busy) {
                return true;
                break;
            }
            i++;
        }
        break;
    }
    return false;
}

void issueInstruction(const Instruction& inst) {
    int ind;
    if (!canIssue(inst, ind))
        return;
    int rbInd = rob.allocate(inst.opcode, inst.dst);
    int16_t val1 = -1, val2 = -1;
    if (inst.src1 >= 0 && !rob.findVal(inst.src1, val1))
        val1 = registers[inst.src1];
    if (inst.src2 >= 0 && !rob.findVal(inst.src2, val2))
        val2 = registers[inst.src2];
    switch (inst.opcode) {
    case 'l':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], -1, rbInd, cycles_num[0], inst.imm);
        regStatus[inst.dst] = ind;
        break;
    case 't':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[1], inst.imm);
        stores[rbInd][0] = -2;
        break;
    case 'b':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[2], inst.pc + inst.imm + 1);
        break;
    case 'c':
        reservationStations[ind] = RSEntry(true, inst.opcode, pc, val2, -1, -1, rbInd, cycles_num[3], inst.pc + inst.imm);
        regStatus[inst.dst] = ind;
        break;
    case 'r':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], -1, rbInd, cycles_num[3], inst.imm);
        break;
    case 'a':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[4], inst.imm);
        regStatus[inst.dst] = ind;
        break;
    case 's':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[4], inst.imm);
        regStatus[inst.dst] = ind;
        break;
    case 'n':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[5], inst.imm);
        regStatus[inst.dst] = ind;
        break;
    case 'm':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[6], inst.imm);
        regStatus[inst.dst] = ind;
        break;
    }
    regStatus[0] = 0;
    pc++;
}


// Phase 3: Execute

bool canLoad(int robId, int address, int& val, bool& other) {
    vector<bool> sts(ROBSize, false);
    for (int i = 0; i < ROBSize; i++)
        if (stores[i][0] == -2 || stores[i][0] == address)
            sts[i] = true;
    int ans = rob.chooseStore(sts, robId);
    if (ans == robId)
        return true;
    if (stores[ans][1]) {
        val = stores[ans][2];
        other = true;
        return true;
    }
    else {
        return false;
    }
}

//void startExecutionForReadyRS();

void decrementExecutionTimers() {
    for (auto rs : reservationStations) {
        if (rs.busy && rs.executionCyclesLeft > 0 && rs.Qj == -1 && rs.Qk == -1) {
            rs.executionCyclesLeft--;
        }
        if (rs.op == 'l' && rs.executionCyclesLeft == 0 && rs.Qk != -2) {
            bool other = false;
            int val;
            if (!canLoad(rs.robIndex, rs.address + rs.Vj, val, other))
                rs.executionCyclesLeft = 1;                 // wait another cycle
            else {
                if (other) {                                   // USE THE EMPTY Qk AND Vk TO GET THINGS FROM STORES
                    rs.Vk = val;
                    rs.Qk = -2;
                }
                else
                {
                    rs.executionCyclesLeft = ReadMemoryTime;
                    rs.Vk = dataMemory[rs.Vj];
                    rs.Qk = -2;
                }
            }
        }
    }
}


// Phase 4: Write-back

void writeBackResults() {
    vector<int> ready(TotalReserveStations, -1);
    for (int i = 0; i < TotalReserveStations;i++) {
        if (reservationStations[i].busy && reservationStations[i].executionCyclesLeft == 0)
            ready[i] = reservationStations[i].robIndex;
    }
    int index = rob.getFirst(ready);
    for (auto st : regStatus)
        if (st == index)
            st = -1;

    int16_t value = -1;
    switch (reservationStations[index].op) {
    case 'l':
        if (reservationStations[index].Qk == -2) {
            value = reservationStations[index].Vk;
        }
        else
            value = reservationStations[index].address + reservationStations[index].Vj;
        break;
    case 't':
        value = reservationStations[index].Vk;
        stores[reservationStations[index].robIndex][0] = reservationStations[index].address + reservationStations[index].Vj;
        stores[reservationStations[index].robIndex][1] = 1;
        stores[reservationStations[index].robIndex][2] = value;
        rob.changeDest(reservationStations[index].robIndex, stores[reservationStations[index].robIndex][0]);
    case 'b':
        value = (reservationStations[index].Vj == reservationStations[index].Vk);
        rob.changeDest(reservationStations[index].robIndex, reservationStations[index].address);
        break;
    case 'c':
        value = reservationStations[index].Vj + 1;
        rob.changeDest(reservationStations[index].robIndex, reservationStations[index].address);
        break;
    case 'r':
        rob.changeDest(reservationStations[index].robIndex, reservationStations[index].Vj);
        break;
    case 'a':
        value = reservationStations[index].Vj + reservationStations[index].Vk;
        break;
    case 's':
        value = reservationStations[index].Vj - reservationStations[index].Vk;
        break;
    case 'n':
        value = ~(reservationStations[index].Vj & reservationStations[index].Vk);
        break;
    case 'm':
        value = reservationStations[index].Vj * reservationStations[index].Vk;
        break;
    }
    for (auto rs : reservationStations) {
        if (rs.busy) {
            if (rs.Qj == index) {
                rs.Qj = -1;
                rs.Vj = value;
            }
            if (rs.Qk == index && rs.op != 'l') {
                rs.Qk = -1;
                rs.Vk = value;
            }
        }
    }
    rob.markReady(reservationStations[index].robIndex, value);
    if (reservationStations[index].op != 's')
        reservationStations[index].busy = false;
}


// Phase 5: Commit

int commitLater = -1;            // has entries that need to be freed after data is written to the memory in WriteMemoryTime cycles
// 2 entries: reservation stage index, when? (how many cycles left)


void commitInstruction() {
    int front;
    commitLater--;
    if (!rob.canCommit(front) || commitLater > 0)
        return;
    int16_t dest = rob.getDest(front);
    if (commitLater == 0) {
        for (int i = 0; i < TotalReserveStations; i++)
            if (reservationStations[i].robIndex == dest)
                reservationStations[i].busy = false;
        rob.commit();
    }
    commitLater = -1;
    pair<int, int> typevalue;
    typevalue = rob.getData(front);
    switch (typevalue.first) {
    case 'l':
        registers[dest] = typevalue.second;
    case 't':
        dataMemory[dest] = typevalue.second;
        commitLater = WriteMemoryTime;
    case 'b':
        if (typevalue.second)
            pc = dest;
            flushPipeline();
        break;
    case 'c':
        pc = dest;
        registers[1] = typevalue.second;
        flushPipeline();
        break;
    case 'r':
        pc = dest;
        flushPipeline();
        break;
    case 'a':
        registers[dest] = typevalue.second;
        break;
    case 's':
        registers[dest] = typevalue.second;
        break;
    case 'n':
        registers[dest] = typevalue.second;
        break;
    case 'm':
        registers[dest] = typevalue.second;
        break;
    }
    registers[0] = 0;
    if(typevalue.first != 's')
        rob.commit();
}

void flushPipeline() {   // For branch misprediction
    for (auto rs : reservationStations) {
        rs.busy = false;
    }
    rob.flushAfter();
}

// Phase 6: Statistics / Logging

void recordIssue(int instID);
void recordExecStart(int instID);
void recordExecEnd(int instID);
void recordWrite(int instID);
void recordCommit(int instID);
void printResults();


// Phase 7: Simulator Loop

void runSimulator() {
    while (!rob.isEmpty()) {
        commitInstruction();
        writeBackResults();
        //startExecutionForReadyRS();
        decrementExecutionTimers();
        if (pc<programMemory.size())
            issueInstruction(programMemory[pc]);
        cycle++;
    }

    printResults();
}


// Main

int main() {
    loadProgram();
    initRegisters();
    initMemory();
    initReservationStations();

    runSimulator();

    return 0;
}
