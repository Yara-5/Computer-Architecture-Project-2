#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

using namespace std;


// Global Constants and Types

const int NUM_REGS = 8;
const int MEMORY_SIZE = 65536;  // 128 KB word-addressable

struct Instruction {
    string opcode;
    int dst, src1, src2;
    int16_t imm;       // offset or immediate
    int pc;            // program counter
    int labelTarget;   // for CALL/BEQ
};

// ROB entry
struct ROBEntry {
    bool busy;
    string type;
    int destination;     // register or memory address
    int16_t value;
    bool ready;
};

// Reservation Station entry
struct RSEntry {
    bool busy;
    string op;
    int16_t Vj, Vk;
    int Qj, Qk;
    int robIndex;
    int executionCyclesLeft;
    int16_t address;   // for load/store
};


// Global Simulator State

vector<Instruction> programMemory;         // Instructions
int16_t dataMemory[MEMORY_SIZE];           // Data memory
int16_t registers[NUM_REGS];               // Register file
int regStatus[NUM_REGS];                   // ROB index producing reg (-1 if free)

vector<ROBEntry> ROB;                       // Reorder buffer
vector<RSEntry> reservationStations;        // All RS entries

int pc = 0;                                 // Program counter
int cycle = 0;                              // Global cycle counter


// Phase 1: Initialization

void loadProgram();                          // Load instructions into programMemory
void initRegisters();                        // Initialize registers and regStatus
void initMemory();                           // Initialize dataMemory
void initROB();                              // Initialize ROB
void initReservationStations();              // Initialize all RS entries


// Phase 2: Issue

bool canIssue(const Instruction& inst);
void issueInstruction(const Instruction& inst);


// Phase 3: Execute

void startExecutionForReadyRS();
void decrementExecutionTimers();


// Phase 4: Write-back

void writeBackResults();


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
    while (/* ROB not empty or instructions left */) {
        commitInstruction();
        writeBackResults();
        startExecutionForReadyRS();
        decrementExecutionTimers();
        if (/* instructions left to issue */)
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
    initROB();
    initReservationStations();

    runSimulator();

    return 0;
}
