#ifndef PROCESS_H
#define PROCESS_H

#include <memory>
#include <iostream>
#include <vector>
#include <array>
using namespace std;

class Process
{
public:
  // making data member public to simplify the code (anti pattern)

  // Assume each process have its own virtual address space of exactly 64 virtual pages => only need 1 byte to store the value
  // spec for each VMA: start_vpage, end_vpage, write_protected[0/1], file_mapped[0/1]
  // NOTE: char range from -128 ~ 127, can do arithematic operations directly, but need to convert to int when print out
  // TODO: use a struct of char & bool for VMA
  unique_ptr<vector<unique_ptr<array<char, 4>>>> VMAs;

  Process(unique_ptr<vector<unique_ptr<array<char, 4>>>> &);
};

// TODO: should move the implementation to ~.cpp after making the makefile that compiles ~.cpp
Process::Process(unique_ptr<vector<unique_ptr<array<char, 4>>>> &VMAs)
    : VMAs(move(VMAs))
{
}

std::ostream &operator<<(std::ostream &os, const unique_ptr<Process> &proc)
{
  for (const unique_ptr<array<char, 4>> &VMA : *(proc->VMAs))
  {
    os << static_cast<int>((*VMA)[0]) << " "
       << static_cast<int>((*VMA)[1]) << " "
       << static_cast<int>((*VMA)[2]) << " "
       << static_cast<int>((*VMA)[3]) << endl;
  }
  return os;
}

#endif