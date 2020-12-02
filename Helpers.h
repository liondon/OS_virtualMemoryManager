#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <regex>
using namespace std;

#include "Process.h"

// get_next_instruction(&operation, &vpage)
// {
// }

enum class ExpectL : char
{
  ProcessCount,
  VMAsCount,
  VMASpec,
};

vector<unique_ptr<Process>> createProcs(ifstream &inputfile)
{
  // Delimiters are spaces (\s) and/or commas
  regex delimiter("[\\s]+");
  string str;
  vector<unique_ptr<Process>> procs;
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
          procs[currProc++] = move(make_unique<Process>(VMAs));

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
