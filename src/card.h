#ifndef __CARD_H__
#define __CARD_H__

#include <string>
#include <uuid/uuid.h>

using namespace std;

class Card final {
  string name{};
  int cardNum{};
  long unsigned int mobileNum{};
  string address{};
  string emailId{};

 public:
  Card();
  void registration(string name, string number, string address, string email);

  void benefits();
};

static bool isCardValid(uid_t);

#endif