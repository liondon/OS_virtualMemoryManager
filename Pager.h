#ifndef PAGER_H
#define PAGER_H

#include <memory>
#include <vector>
using namespace std;

#include "Frame.h"

class Pager
{
public:
  virtual shared_ptr<Frame> select_victim_frame() const = 0;
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
  mutable int hand = 0; // the "hand"
  // we can define data members that are for all derived class here.
};

///////////////////// E S C / N R U ///////////////////////////
class ESC : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame() const override;
  ESC();

private:
  mutable int resetInstCount;
};

ESC::ESC()
    : Pager('e'), resetInstCount(0)
{
}

shared_ptr<Frame> ESC::select_victim_frame() const
{
  hand = hand > MAX_FRAME ? 0 : hand;
  int counter = 0;
  bool resetRbits = (instCount - resetInstCount) >= 50;
  array<int, 4> cls;
  for (int &c : cls)
  {
    c = -1;
  }

  if (resetRbits)
  { // update resetInstCount
    resetInstCount = instCount;
  }

  int tail = hand;
  do
  {
    counter++;
    shared_ptr<Process> proc = frame_table[hand]->proc;
    pte_t &pte = proc->page_table[frame_table[hand]->vPageId];
    int idx = pte.referenced * 2 + pte.modified;

    if (cls[idx] == -1)
    {
      cls[idx] = hand; // frame_i is the first frame we encountered for class_x
      if (idx == 0 && !resetRbits)
      {
        // cout << "ASELECT : " << tail << " "
        //      << resetRbits << " | "
        //      << idx << " "
        //      << cls[idx] << " "
        //      << counter;
        return frame_table[hand++];
      }
    }

    if (resetRbits)
    {
      // reset R bit
      pte.referenced = 0;
    }

    hand++;
    hand = hand > MAX_FRAME ? 0 : hand;
  } while (hand != tail);

  int idx = 0;
  for (; idx < 4; idx++)
  {
    hand = cls[idx];
    if (hand != -1)
    {
      // cout << "ASELECT : " << (tail == MAX_FRAME ? 0 : tail + 1) << " "
      //      << resetRbits << " | "
      //      << idx << " "
      //      << cls[idx] << " "
      //      << counter;
      return frame_table[hand++];
    }
  }

  // cout << "ASELECT : " << (tail == MAX_FRAME ? 0 : tail + 1) << " "
  //      << resetRbits << " | "
  //      << idx << " "
  //      << "nullptr "
  //      << counter;
  return nullptr;
}
/////////////////////////////////////////////////////////

///////////////////// C L O C K ///////////////////////////
class CLCK : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame() const override;
  CLCK();

private:
};

CLCK::CLCK()
    : Pager('C')
{
}

shared_ptr<Frame> CLCK::select_victim_frame() const
{
  while (true)
  {
    if (hand > MAX_FRAME)
    {
      // cout << hand << " > MAX_FRAME=" << static_cast<int>(MAX_FRAME) << endl;
      hand = 0;
    }
    shared_ptr<Process> proc = frame_table[hand]->proc;
    char vPageId = frame_table[hand]->vPageId;
    pte_t &pte = proc->page_table[vPageId];
    if (pte.referenced)
    {
      pte.referenced = 0;
      hand++;
    }
    else
    {
      return frame_table[hand++];
    }
  }
}
/////////////////////////////////////////////////////////

///////////////////// R A N D O M ///////////////////////////
// TODO: how to put the helper functions to Helpers.h?
int myrandom(const vector<int> *);
vector<int> *createRandArray(const string);

class RAND : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame() const override;
  RAND(const string);

private:
  vector<int> *randArray;
};

RAND::RAND(const string randPath)
    : Pager('R'), randArray(createRandArray(randPath))
{
}

shared_ptr<Frame> RAND::select_victim_frame() const
{
  int r = myrandom(randArray);
  return frame_table[r];
}
/////////////////////////////////////////////////////////

///////////////////// F I F O ///////////////////////////
class FIFO : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame() const override;
  FIFO();

private:
};

FIFO::FIFO()
    : Pager('F')
{
}

shared_ptr<Frame> FIFO::select_victim_frame() const
{
  if (hand > MAX_FRAME)
  {
    // cout << hand << " > MAX_FRAME=" << static_cast<int>(MAX_FRAME) << endl;
    hand = 0;
  }
  return frame_table[hand++];
}
/////////////////////////////////////////////////////////

int myrandom(const vector<int> *randArray)
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
