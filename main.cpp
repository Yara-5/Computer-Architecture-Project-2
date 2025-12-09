
#include "ROB.cpp"


// Global Constants and Types

const int NUM_REGS = 8;
const int MEMORY_SIZE = 65536;  // 128 KB word-addressable

// Station Constants

int reserve_num[] = {2, 1, 2, 1, 4, 2, 1};
int reserve_start[7];
int cycles_num[] = { 2,2,1,1,2,1,12 };
int TotalReserveStations;

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

ROB rob(8);                                 // Initializing Reorder buffer
vector<RSEntry> reservationStations;        // All RS entries

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
        break;
    case 't':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[1], inst.imm);
        break;
    case 'b':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[2], inst.pc + inst.imm + 1);
        break;
    case 'c':
        reservationStations[ind] = RSEntry(true, inst.opcode, pc, val2, -1, -1, rbInd, cycles_num[3], inst.pc + inst.imm);
        break;
    case 'r':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], -1, rbInd, cycles_num[3], inst.imm);
        break;
    case 'a':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[4], inst.imm);
        break;
    case 's':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[4], inst.imm);
        break;
    case 'n':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[5], inst.imm);
        break;
    case 'm':
        reservationStations[ind] = RSEntry(true, inst.opcode, val1, val2, regStatus[inst.src1], regStatus[inst.src2], rbInd, cycles_num[6], inst.imm);
        break;
    }
    pc++;
}


// Phase 3: Execute

//void startExecutionForReadyRS();

void decrementExecutionTimers() {
    for (auto rs : reservationStations) {
        if (rs.busy && rs.executionCyclesLeft > 0 && rs.Qj == -1 && rs.Qk == -1) {
            rs.executionCyclesLeft--;
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
        value = dataMemory[reservationStations[index].address + reservationStations[index].Vj];
        break;
    case 't':
        value = reservationStations[index].Vk;
        rob.changeDest(reservationStations[index].robIndex, reservationStations[index].address + reservationStations[index].Vj);
    case 'b':
        value = (reservationStations[index].Vj == reservationStations[index].Vk);
        rob.changeDest(reservationStations[index].robIndex, reservationStations[index].address);
        break;
    case 'c':
        value = reservationStations[index].Vj + reservationStations[index].Vk;
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
        if (rs.Qj == index) {
            rs.Qj = -1;
            rs.Vj = value;
        }
        if (rs.Qk == index) {
            rs.Qk = -1;
            rs.Vk = value;
        }
    }
    rob.markReady(reservationStations[index].robIndex, value);
}


// Phase 5: Commit

void commitInstruction();
void flushPipeline();   // For branch misprediction


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
