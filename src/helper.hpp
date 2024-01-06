#include <iostream>
#include <ctime>

void creditPurchase(int ticketPrice) {
  time_t t = time(NULL);
  tm* timePtr = localtime(&t);
  std::cout << "Paying :" << ticketPrice << "\n";
  int expirymm, expiryyy;
  std::string name{}, cardNum{};
  std::cout << "Name of the card holder";
  std::cin >> name;
  std::cout << "Enter the card number";
  std::cin >> cardNum;
  std::cout << "Expiry(MM/YYYY)";
  std::cin >> expirymm;
  std::cout << "/";
  std::cin >> expiryyy;
  while (expirymm < (timePtr->tm_mon) || expiryyy < (timePtr->tm_year + 1900)) {
    if (expirymm <= (timePtr->tm_mon)) {
      std::cout << "Enter the month again";
      std::cin >> expirymm;
    }
    if (expiryyy < (timePtr->tm_year + 1900)) {
      std::cout << "Please enter a valid year";
      std::cin >> expiryyy;
    }
  };
}


void hidePassword() {
  char password[3], vh;
  int h;
  puts("Enter the CVV/CVV2");
  while (1) {
    if (h < 0) h = 0;
    vh = getch();
    if (vh == 13) break;
    if (vh == 8) {
      putch(NULL);
      putch(NULL);
      putch(NULL);
      h--;
      continue;
    }
    password[h++] = vh;
    vh = '*';
    putch(vh);
  };
  password[h] == '\0';
}

void getCurrentTime() {
  time_t t = time(NULL);
  tm* timePtr = localtime(&t);
  std::cout << "Time of the computer presently:";
  std::cout << "seconds= " << timePtr->tm_sec << std::endl;
  std::cout << "minutes = " << timePtr->tm_min << std::endl;
  std::cout << "hours = " << timePtr->tm_hour << std::endl;
  std::cout << "day of month = " << timePtr->tm_mday << std::endl;
  std::cout << "month of year = " << timePtr->tm_mon << std::endl;
  std::cout << "year = " << timePtr->tm_year + 1900 << std::endl;
  std::cout << "weekday = " << timePtr->tm_wday << std::endl;
  std::cout << "day of year = " << timePtr->tm_yday << std::endl;
  std::cout << "daylight savings = " << timePtr->tm_isdst << std::endl;
}