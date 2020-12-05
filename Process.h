#ifndef PROCESS_H
#define PROCESS_H

#include <memory>
#include <iostream>
#include <vector>
#include <array>
using namespace std;

#define NUM_OF_VPAGES 64

typedef struct pte_t
{
  // can only be total of 32-bit size
  // PTE is comprised of the PRESENT/VALID, REFERENCED, MODIFIED, WRITE_PROTECT, PAGEDOUT and the number of physical frame
  // You can use other bits at will (e.g. remembering whether a PTE is file mapped or not).
  /* What you can NOT do is run at the beginning of the program through the page_table and mark each PTE
  ** This is NOT how OSes do this due to hierarchical pagetable structures (not implemented in this lab though). 
  ** You can only set those bits on the first page fault to that virtual page. */
  unsigned int valid : 1;
  unsigned int referenced : 1;
  unsigned int modified : 1;
  unsigned int write_protect : 1;
  unsigned int pageout : 1;
  unsigned int frame : 7;

  unsigned int file_mapped : 1;
  unsigned int initialized : 1;
  unsigned int accessible : 1;
  // unsigned int padding : N;
  // N would be 32 - <sum of all the other bits>
  // In general, when you work with hardware in real world,
  // you have to layout your datastructure exactly how the hardware specifies it.
  // So padding is often the case.

  pte_t() : valid(0), referenced(0), modified(0), write_protect(0), pageout(0), frame(0),
            file_mapped(0), initialized(0), accessible(0)
  {
  }

  pte_t(bool v, bool r, bool m, bool w, bool p, bool f, bool fm, bool i, bool a)
      : valid(v), referenced(r), modified(m), write_protect(w), pageout(p), frame(f),
        file_mapped(fm), initialized(i), accessible(a) {}
} pte_t;

typedef struct pstats
{
  // can only be total of 32-bit size
  // PTE is comprised of the PRESENT/VALID, REFERENCED, MODIFIED, WRITE_PROTECT, PAGEDOUT and the number of physical frame
  // You can use other bits at will (e.g. remembering whether a PTE is file mapped or not).
  /* What you can NOT do is run at the beginning of the program through the page_table and mark each PTE
  ** This is NOT how OSes do this due to hierarchical pagetable structures (not implemented in this lab though). 
  ** You can only set those bits on the first page fault to that virtual page. */
  int unmaps, maps, ins, outs, fins, fouts, zeros, segv, segprot;

  pstats()
      : unmaps(0), maps(0), ins(0), outs(0), fins(0), fouts(0),
        zeros(0), segv(0), segprot(0) {}
} pstats;

class Process
{
public:
  // making data member public to simplify the code (anti pattern)

  // Assume each process have its own virtual address space of exactly 64 virtual pages => only need 1 byte to store the value
  // spec for each VMA: start_vpage, end_vpage, write_protected[0/1], file_mapped[0/1]
  // NOTE: char range from -128 ~ 127, can do arithematic operations directly, but need to convert to int when print out
  // TODO: use a struct of char & bool for VMA? Make it more semantic.
  inline static int i;
  int id;
  unique_ptr<vector<unique_ptr<array<char, 4>>>> VMAs;
  vector<pte_t> page_table;
  pstats pstats;

  Process(unique_ptr<vector<unique_ptr<array<char, 4>>>> &);
};

// TODO: should move the implementation to ~.cpp after making the makefile that compiles ~.cpp
Process::Process(unique_ptr<vector<unique_ptr<array<char, 4>>>> &VMAs)
    : id(i++), VMAs(move(VMAs)), page_table(NUM_OF_VPAGES), pstats()
{
}

std::ostream &operator<<(std::ostream &os, const shared_ptr<Process> &proc)
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