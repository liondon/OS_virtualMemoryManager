#include <memory>
#include <fstream>
#include <iostream>
#include <vector>
#include <deque>
#include <string>
using namespace std;

#include "Frame.h"
#include "Process.h"
#include "Helpers.h"
#include "Pager.h"

int main()
{
  // TODO: take from input args
  const string inputPath = "./testdata/in3";
  const char MAX_FRAME = 16 - 1; // 0 ~ 127, maximum 128 frames
  const Pager *pager = new FIFO();
  bool O = 0, P = 1, F = 1, S = 1;

  ifstream inputfile;
  inputfile.open(inputPath);
  vector<shared_ptr<Process>> procs = createProcs(inputfile); // processes are numbered starting from 0
  shared_ptr<Process> current_process = nullptr;

  int instCount = 0, ctxSwitches = 0, processExits = 0;
  unsigned long long totalCycles = 0;
  string operation;
  int target;

  // initialize frame_table and freePool
  shared_ptr<Frame> frame_table[MAX_FRAME + 1];
  deque<shared_ptr<Frame>> freePool;
  for (char i = 0; i <= MAX_FRAME; i++)
  {
    shared_ptr<Frame> f(new Frame(i));
    frame_table[i] = f;
    freePool.emplace_back(f);
  }

  while (get_next_instruction(inputfile, operation, target))
  {
    cout << instCount++ << ": ==> " << operation << " " << target << endl;
    // handle special case of "c" and "e" instruction
    // "c <procid>":  context switch to process #<procid>
    if (operation == "c")
    {
      current_process = procs[target];
      ctxSwitches++;
      totalCycles += 121;
      continue;
    }
    else if (operation == "e")
    {
      // TODO: "e <procid>": current process (#<procid>) exits
      processExits++;
      totalCycles += 175;
      continue;
    }

    // now the real instructions for read and write
    // "r <vpage>": a load/read operation on <vpage> of current process.
    // "w <vpage>": a store/write operation on <vpage> of current process.
    pte_t &pte = current_process->page_table[target]; // in reality this is done by hardware
    if (!pte.valid)
    { // **** Page Fault Exception Handler ****

      // 1. Determine whether the vpage can be accessed, i.e. is it part of one of the VMAs?
      /* find a faster way then searching each time the VMAs list (Hint: free bits in the PTE) */
      if (!pte.initialized)
      { // First time trying to access this vPage
        bool isAccessible = 0, isWriteProtect = 0, isFileMapped = 0;
        for (unique_ptr<array<char, 4>> &VMA : *(current_process->VMAs))
        {
          if ((*VMA)[0] <= target && target <= (*VMA)[1])
          {
            isAccessible = 1;
            isWriteProtect = (*VMA)[2];
            isFileMapped = (*VMA)[3];
          }
        }
        pte = {0, 0, 0, isWriteProtect, 0, 0, isFileMapped, 1, isAccessible};
        // valid, referenced, modified, write_protect, pageout, frame, file_mapped, initialized, accessible
      }
      if (!pte.accessible)
      { // It is not a part of a VMA:
        // a SEGV output line must be created AND move on to the next instruction
        cout << " SEGV" << endl;
        current_process->pstats.segv++;
        totalCycles += 240;
        continue;
      }

      // If it is part of a VMA, then the page must be instantiated:
      // => a frame must be allocated, assigned to the PTE belonging to the vpage of this instruction
      shared_ptr<Frame> newframe = get_frame(MAX_FRAME, frame_table, freePool,
                                             pager, current_process, totalCycles);

      // TODO: reset the PTE (M/R bits only?) What's the rule here?
      // NOTE: once the PAGEDOUT flag is set, it will never be reset as it indicates there is content on the swap device
      pte.referenced = 0;
      pte.modified = 0;

      // maps it: set the PTE's frame and valid bits
      pte.frame = newframe->id;
      pte.valid = 1;

      // update the frame table: The frame table can only be accessed as part of the "simulated page fault handler"
      newframe->proc = current_process;
      newframe->vPageId = target;

      // populated with the proper content, depends whether this page was previously paged out
      /* If paged out: the page must be brought back from the swap space ("IN") */
      if (pte.pageout)
      {
        cout << " IN" << endl;
        current_process->pstats.ins++;
        totalCycles += 3000;
      }
      /* If it is a memory mapped file: "FIN" */
      if (pte.file_mapped)
      {
        cout << " FIN" << endl;
        current_process->pstats.fins++;
        totalCycles += 2500;
      }
      /* If the vpage was never swapped out and is not file mapped, then by definition it still has a zero filled content and
      ** you issue the "ZERO" output.*/
      if (!pte.pageout && !pte.file_mapped)
      {
        cout << " ZERO" << endl;
        current_process->pstats.zeros++;
        totalCycles += 150;
      }

      cout << " MAP " << static_cast<int>(newframe->id) << endl;
      current_process->pstats.maps++;
      totalCycles += 400;
    }

    // Update the  PTE's R/M bits to simulate instruction execution by hardware,
    // i.e. set the REFERENCED and MODIFIED bits based on the operation
    if (operation == "w")
    {
      totalCycles += 1;
      if (pte.write_protect)
      {
        // If the instruction is a write operation and the PTE's write protect bit is set (inherited from the VMA it belongs to)
        // then output "SEGPROT". The page is considered referenced but not modified in this case.
        cout << " SEGPROT" << endl;
        current_process->pstats.segprot++;
        totalCycles += 300;
        pte.referenced = 1;
      }
      else
      {
        pte.modified = 1;
        pte.referenced = 1; //TODO: modified => referenced?
      }
    }
    else if (operation == "r")
    {
      totalCycles += 1;
      pte.referenced = 1;
    }
  }

  inputfile.close();

  // Debug only: print out processes
  for (size_t i = 0; i < procs.size(); i++)
  {
    cout << "#### process " << i << endl
         << "#" << endl
         << procs[i]->VMAs->size() << endl
         << procs[i];
  }

  if (P)
  { // Print page table of each processes
    for (size_t i = 0; i < procs.size(); i++)
    {
      shared_ptr<Process> &proc = procs[i];
      cout << "PT[" << i << "]:";
      for (size_t i = 0; i < NUM_OF_VPAGES; i++)
      {
        pte_t &pte = proc->page_table[i];
        if (!pte.valid)
        {
          string p = pte.pageout ? " #" : " *";
          cout << p;
          continue;
        }
        string r = pte.referenced ? "R" : "-";
        string m = pte.modified ? "M" : "-";
        string s = pte.pageout ? "S" : "-";
        cout << " " << i << ":" << r << m << s;
      }
      cout << endl;
    }
  }

  if (F)
  { // Print frame table
    cout << "FT:";
    for (int i = 0; i <= MAX_FRAME; i++)
    {
      shared_ptr<Frame> frame = frame_table[i];
      cout << " " << frame->proc->id << ":" << static_cast<int>(frame->vPageId);
    }
    cout << endl;
  }

  if (S)
  { // Print the summary statistics
    // Per process output:
    for (auto proc : procs)
    {
      pstats *pstats = &(proc->pstats);
      printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
             proc->id,
             pstats->unmaps, pstats->maps, pstats->ins, pstats->outs, pstats->fins, pstats->fouts,
             pstats->zeros, pstats->segv, pstats->segprot);
    }
    // Summary output:
    printf("TOTALCOST %lu %lu %lu %llu\n", instCount, ctxSwitches, processExits, totalCycles);
  }

  return 0;
}
