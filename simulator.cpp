#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "Helpers.h"
#include "Process.h"

int main()
{
  // TODO: take from input args
  int frameNum = 128;
  string inputPath = "in9";

  ifstream inputfile;
  inputfile.open(inputPath);
  vector<unique_ptr<Process>> procs = createProcs(inputfile); // processes are numbered starting from 0

  // while (get_next_instruction(&operation, &vpage))
  // {
  //   // handle special case of “c” and “e” instruction
  //   // now the real instructions for read and write
  //   pte_t *pte = &current_process.page_table[vpage]; // in reality this is done by hardware
  //   if (!pte->present)
  //   {
  //     // this in reality generates the page fault exception and now you execute
  //     // verify this is actually a valid page in a vma if not raise error and next inst
  //     frame_t *newframe = get_frame();

  //     //-> figure out if/what to do with old frame if it was mapped
  //     //   see general outline in MM-slides under Lab3 header
  //     //   see whether and how to bring in the content of the access page.
  //   }
  //   // check write protection
  //   // simulate instruction execution by hardware by updating the R/M PTE bits
  //   update_pte(read / modify) bits based on operations.
  // }

  inputfile.close();

  // Debug only: print out processes
  for (size_t i = 0; i < procs.size(); i++)
  {
    cout << "#### process " << i << endl
         << "#" << endl
         << procs[i]->VMAs->size() << endl
         << procs[i];
  }

  return 0;
}
