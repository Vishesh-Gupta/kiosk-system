#ifndef __CARD_H__
#define __CARD_H__

#include <string>
#include <uuid/uuid.h>

class Card final {
  std::string name{};
  int cardNum{};
  long unsigned int mobileNum{};
  std::string address{};
  std::string emailId{};

 public:
  Card();
  void registration();

  void benefits();
};

static bool isCardValid(uid_t);

#endif