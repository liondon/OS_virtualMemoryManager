#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <regex>
using namespace std;

#include "Process.h"
#include "Frame.h"
#include "Pager.h"

// TODO: should move the implementation to ~.cpp after making the makefile that compiles ~.cpp

shared_ptr<Frame> get_frame(const char MAX_FRAME, shared_ptr<Frame> frame_table[],
                            deque<shared_ptr<Frame>> &freePool, const Pager *pager,
                            shared_ptr<Process> &current_process, unsigned long long &totalCycles)
{
  shared_ptr<Frame> frame = nullptr;
  if (!freePool.empty())
  {
    frame = freePool.front();
    // for (auto frame : freePool)
    // {
    //   cout << "DEBUG: freePool " << static_cast<int>(frame->id) << endl;
    // }

    freePool.pop_front();
    // for (auto frame : freePool)
    // {
    //   cout << "DEBUG: freePool " << static_cast<int>(frame->id) << endl;
    // }
  }

  if (frame == nullptr)
  {
    frame = pager->select_victim_frame(MAX_FRAME, frame_table);
    cout << " UNMAP " << frame->proc->id << ":" << static_cast<int>(frame->vPageId) << endl;
    current_process->pstats.unmaps++;
    totalCycles += 400;
    pte_t &pte = frame->proc->page_table[frame->vPageId];
    // TODO: UNMAP victim_frame's original vPage: update PTE? What else than the valid bit?
    pte.valid = 0;

    // If the page was dirty (modified) (tracked in the PTE)
    // it pages the page OUT to a swap device with the (1:26) tag so the OS can find it later
    // when process 1 references vpage 42 again (NOTE: you don’t implement the lookup)
    // in case it was file mapped written back to the file ("FOUT")
    if (pte.modified)
    {
      if (pte.file_mapped)
      {
        cout << " FOUT" << endl;
        current_process->pstats.fouts++;
        totalCycles += 2500;
      }
      else
      {
        cout << " OUT" << endl;
        current_process->pstats.outs++;
        totalCycles += 3000;
        pte.pageout = 1;
      }
    }
  }
  // cout << "DEBUG: selected frame = " << static_cast<int>(frame->id) << endl;
  return frame;
}

bool get_next_instruction(ifstream &inputfile, string &operation, int &target)
{
  // Delimiters are spaces (\s) and/or commas
  regex delimiter("[\\s]+");
  string str;

  while (getline(inputfile, str))
  {
    if (inputfile.is_open())
    {
      if (*str.begin() == '#')
      {
        // all lines starting with '#' must be ignored
        continue;
      }
      vector<string> tokens(sregex_token_iterator(str.begin(), str.end(), delimiter, -1), {});
      operation = tokens[0];
      target = stoi(tokens[1]);
      return true;
    }
  }
  return false;
}

enum class ExpectL : char
{
  ProcessCount,
  VMAsCount,
  VMASpec,
};

vector<shared_ptr<Process>> createProcs(ifstream &inputfile)
{
  // Delimiters are spaces (\s) and/or commas
  regex delimiter("[\\s]+");
  string str;
  vector<shared_ptr<Process>> procs;
  unique_ptr<vector<unique_ptr<array<char, 4>>>> VMAs;
  ExpectL exp = ExpectL::ProcessCount;
  int processCount = 0, VMAsCount = 0, currProc = 0, currVMA = 0;

  while (getline(inputfile, str))
  {
    if (inputfile.is_open())
    {
      if (*str.begin() == '#')
      {
        // all lines starting with '#' must be ignored
        continue;
      }

      vector<string> tokens(sregex_token_iterator(str.begin(), str.end(), delimiter, -1), {});

      if (exp == ExpectL::ProcessCount)
      {
        processCount = stoi(tokens[0]);
        procs.resize(processCount);
        exp = ExpectL::VMAsCount;
      }
      else if (exp == ExpectL::VMAsCount)
      {
        VMAsCount = stoi(tokens[0]);
        VMAs = make_unique<vector<unique_ptr<array<char, 4>>>>();
        VMAs->resize(VMAsCount);
        exp = ExpectL::VMASpec;
      }
      else if (exp == ExpectL::VMASpec)
      {
        unique_ptr<array<char, 4>> VMA(new array<char, 4>({static_cast<char>(stoi(tokens[0])),
                                                           static_cast<char>(stoi(tokens[1])),
                                                           static_cast<char>(stoi(tokens[2])),
                                                           static_cast<char>(stoi(tokens[3]))}));
        (*VMAs)[currVMA++] = move(VMA);
        if (--VMAsCount == 0)
        {
          currVMA = 0;
          exp = ExpectL::VMAsCount;

          // create Process obj.
          procs[currProc++] = move(make_shared<Process>(VMAs));

          if (currProc == processCount)
          {
            break;
          }
        }
      }
    }
  }

  return procs;
}
