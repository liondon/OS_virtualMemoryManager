#ifndef PAGER_H
#define PAGER_H

#include <memory>
#include <vector>
using namespace std;

#include "Frame.h"

class Pager
{
public:
  virtual shared_ptr<Frame> select_victim_frame(const char, shared_ptr<Frame>[]) const = 0;

  /* Once a victim frame has been determined, 
  ** the victim frame must be unmapped from its user, i.e. its entry in the owning process’s page_table must be removed ("UNMAP"), 
  ** you must inspect the state of the R and M bits when UNMAPPING. 
  ** If the page was modified, then the page frame must be paged out to the swap device ("OUT") or 
  ** if it was file mapped, written back to the file ("FOUT").
  */
protected:
  // we can define data members that are for all derived class here.
};

///////////////////// F I F O ///////////////////////////
class FIFO : public Pager
{
public:
  shared_ptr<Frame> select_victim_frame(const char, shared_ptr<Frame>[]) const override;

private:
};

shared_ptr<Frame> FIFO::select_victim_frame(const char MAX_FRAME, shared_ptr<Frame> frame_table[]) const
{
  static int i = 0;
  if (i > MAX_FRAME)
  {
    // cout << i << " > MAX_FRAME=" << static_cast<int>(MAX_FRAME) << endl;
    i = 0;
  }
  return frame_table[i++];
}

/////////////////////////////////////////////////////////

#endif
