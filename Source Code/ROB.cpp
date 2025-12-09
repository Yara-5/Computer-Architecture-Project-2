#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <limits>
#include <cctype>
using namespace std;

// ROB entry
struct ROBEntry {
    bool busy;
    char type;
    int destination;     // register or memory address
    int16_t value;
    bool ready;

    ROBEntry()
        : busy(false), type(' '), destination(-1), value(0), ready(false) {
    }

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

    int getDest(int index) {
        return entries[index].destination;
    }

    pair<int, int> getData(int index) {
        return make_pair(entries[index].type, entries[index].value);
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
        if (isEmpty())
            return;
        entries[head].busy = false;
        if (count == 1) {
            head = 0;
            tail = 0;
        }
        else
            head = (head + 1) % size;
        count--;
    }

    void flushAfter() {
        while (tail != head)
        {
            count--;
            tail = (tail - 1) % size;
            entries[tail].busy = false;
        }
    }

    //int compare(int i1, int i2) {
    //    int index = head;
    //    while (index != tail) {
    //        if (index == i1)
    //            return i1;
    //        if (index == i2)
    //            return i2;
    //    }
    //    return -1;
    //}

};