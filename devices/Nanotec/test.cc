#include <iostream>

#ifdef USE_FAKEIO
#include "NanotecSMCI36Fake.h"
typedef NanotecSMCI36Fake NanotecSMCI36_t;
#else
#include "NanotecSMCI36.h"
typedef NanotecSMCI36 NanotecSMCI36_t;
#endif

int main()
{
  std::cout << "test" << std::endl;

  NanotecSMCI36_t smci36("/dev/ttyUSB0");

  std::cout << "firmware version:          " << smci36.GetFirmwareVersion() << std::endl;
  std::cout << "status:                    " << std::hex << smci36.GetStatus() << std::endl;

  return 0;
}
