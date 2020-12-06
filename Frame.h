#ifndef FRAME_H
#define FRAME_H

#include <memory>
#include <iostream>
#include <bitset>
using namespace std;

#include "Process.h"

class Frame
{
  // define a global frame_table that each operating system maintains
  // to describe the usage of each of its physical frames and where you maintain reverse mappings to the process and the vpage
  // Note: in this assignment, a frame can only be mapped by at most one PTE at a time, which simplifies things significantly.
public:
  char id;
  shared_ptr<Process> proc;
  char vPageId;
  bitset<32> age;                // for aging algo
  unsigned int lastRefInstCount; // for working set

  Frame(char);

private:
};

Frame::Frame(char i)
    : id(i), proc(nullptr), vPageId(0), age(0), lastRefInstCount(0)
{
}

#endif