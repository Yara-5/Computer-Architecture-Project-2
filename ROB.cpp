#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
using namespace std;

// ROB entry
struct ROBEntry {
    bool busy;
    char type;
    int destination;     // register or memory address
    int16_t value;
    bool ready;

    ROBEntry(char typ, int dest, int val) {
        busy = true;
        type = typ;
        destination = dest;
        value = val;
        ready = false;
    }
};


class ROB {
    int size;           // fixed
    int head;
    int tail;
    vector<ROBEntry> entries;
    int count;          // Number of elements currently in ROB

public:

    ROB(int robSize) {
        head = 0;
        tail = 0;
        count = 0;
        size = robSize;
        entries.resize(robSize);
    }

    bool isFull() const {
        return (count == size);
    }

    bool isEmpty() const {
        return (count == 0);
    }

    int allocate(char type, int dest) {
        if (isFull()) {
            cout << "Error: ROB is full, can't allocate\n";
            return -1;
        }

        entries[tail] = ROBEntry(type, dest, 0);
        int index = tail;
        tail = (tail + 1) % size;
        count++;
        return index;
    }

    bool findVal(int dest, int16_t& value) {
        int index = tail;
        do {
            index = (index - 1) % size;
            if (entries[index].ready && entries[index].type != 's' && entries[index].destination == dest) {
                value = entries[index].value;
                return true;
            }

        } while (index != head);
        return false;
    }

    void markReady(int index, int16_t val) {
        entries[index].ready = true;
        entries[index].value = val;
    }

    void changeDest(int index, int dest) {      // in case of store
        entries[index].destination = dest;
    }

    int getFirst(vector<int> ready) {
        vector<bool> val(size, false);
        for (auto rd : ready)
            if (rd >= 0)
                val[rd] = true;
        int index = head;
        while (index != tail) {
            if (val[index])
                break;
            index = (index + 1) % size;
        }
        int i = 0;
        while (ready[i] != index)
            i++;
        return i;
    }

    int chooseStore(vector<bool> ready, int l) {
        int index = l;
        while (index != head) {
            index = (index - 1) % size;
            if (ready[index])
                return index;
        }
        return l;
    }

    bool canCommit(int& front) const {
        front = head;
        return entries[head].ready;
    }

    void commit() {
        entries[head].busy = false;
        if (count == 1) {
            head = 0;
            tail = 0;
        }
        else
            head = (head + 1) % size;
        count--;
    }

    void flushAfter(int branchIndex) {
        while (tail != branchIndex)
        {
            count--;
            tail = (tail - 1) % size;
            entries[tail].busy = false;
        }
    }

};