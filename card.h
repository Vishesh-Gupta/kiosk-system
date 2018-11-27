#ifndef __CARD_H__
#define __CARD_H__

#include <string>

class Card final {
  std::string name{};
  int cardNum{};
  long unsigned int mobileNum{};
  std::string address{};
  std::string emailId{};

 public:
  Card();
  void cardRegistration();
};

#endif