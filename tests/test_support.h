#ifndef KIOSK_TEST_SUPPORT_H
#define KIOSK_TEST_SUPPORT_H

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "db.h"

// Database-backed tests are skipped when PostgreSQL is not reachable, unless
// KIOSK_REQUIRE_DB=1 is set (as in CI), in which case they fail instead.
inline bool requireDatabase() {
  const char* value = std::getenv("KIOSK_REQUIRE_DB");
  return value != nullptr && std::string(value) == "1";
}

#define KIOSK_SKIP_WITHOUT_DB(db)                                                 \
  do {                                                                            \
    if (!(db).ping()) {                                                           \
      if (requireDatabase()) {                                                    \
        FAIL() << "Database " << (db).getDbName() << "@" << (db).getHost()        \
               << " is unreachable and KIOSK_REQUIRE_DB=1";                       \
      }                                                                           \
      GTEST_SKIP() << "Database " << (db).getDbName() << "@" << (db).getHost()    \
                   << " is unreachable; set DB_* env vars or config/config.json"; \
    }                                                                             \
  } while (0)

#endif  // KIOSK_TEST_SUPPORT_H
