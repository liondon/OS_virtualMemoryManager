#ifndef PAGER_H
#define PAGER_H

#include <memory>
#include <vector>
using namespace std;

#include "Frame.h"

class Pager
{
public:
  virtual shared_ptr<Frame> select_victim_frame(const char, vector<shared_ptr<Frame>> &) const = 0;
  char get_type() { return type; };
  Pager(char c) : type(c){};

  /* Once a victim frame has been determined, 
  ** the victim frame must be unmapped from its user, i.e. its entry in the owning process’s page_table must be removed ("UNMAP"), 
  ** you must inspect the state of the R and M bits when UNMAPPING. 
  ** If the page was modified, then the page frame must be paged out to the swap device ("OUT") or 
  ** if it was file mapped, written back to the file ("FOUT").
  */
protected:
  char type;
  // we can define data members that are for all derived class here.
};

///////////////////// R A N D O M ///////////////////////////
// TODO: how to put the helper functions to Helpers.h?
int myrandom(const int, const vector<int> *);
vector<int> *createRandArray(const string);

class RAND : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame(const char, vector<shared_ptr<Frame>> &) const override;
  RAND(const string);

private:
  vector<int> *randArray;
};

RAND::RAND(const string randPath)
    : Pager('R')
{
  randArray = createRandArray(randPath);
}

shared_ptr<Frame> RAND::select_victim_frame(const char MAX_FRAME, vector<shared_ptr<Frame>> &frame_table) const
{
  // static int i = 0; // the "hand"
  // if (i > MAX_FRAME)
  // {
  //   // cout << i << " > MAX_FRAME=" << static_cast<int>(MAX_FRAME) << endl;
  //   i = 0;
  // }
  int r = myrandom(MAX_FRAME, randArray);
  return frame_table[r];
}
/////////////////////////////////////////////////////////

///////////////////// F I F O ///////////////////////////
class FIFO : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame(const char, vector<shared_ptr<Frame>> &) const override;
  FIFO();

private:
};

FIFO::FIFO()
    : Pager('F')
{
}

shared_ptr<Frame> FIFO::select_victim_frame(const char MAX_FRAME, vector<shared_ptr<Frame>> &frame_table) const
{
  static int i = 0; // the "hand"
  if (i > MAX_FRAME)
  {
    // cout << i << " > MAX_FRAME=" << static_cast<int>(MAX_FRAME) << endl;
    i = 0;
  }
  return frame_table[i++];
}
/////////////////////////////////////////////////////////

int myrandom(const int MAX_FRAME, const vector<int> *randArray)
{
  static int ofs = 0;
  if (ofs >= randArray->size())
  {
    ofs = 0;
  }
  return ((*randArray)[ofs++] % (MAX_FRAME + 1));
}

vector<int> *createRandArray(const string randFilePath)
{
  vector<int> *randArray = new vector<int>();
  ifstream randFile;
  string str;

  randFile.open(randFilePath);

  // get the #randNum in this file
  getline(randFile, str);
  const int amount = stoi(str);

  // read in the randNums
  while (getline(randFile, str))
  {
    if (randFile.is_open())
    {
      randArray->emplace_back(stoi(str));
    }
  }
  randFile.close();

  if (amount != randArray->size())
  {
    // TODO: more appropriate error handling
    cout << "Something wrong with the random file." << endl;
    return nullptr;
  }

  return randArray;
}

#endif
