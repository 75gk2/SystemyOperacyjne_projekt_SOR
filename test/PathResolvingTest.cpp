#include "constants.hpp"
#include "catch2/catch_test_macros.hpp"

#include <fcntl.h>
#include <unistd.h>

#include "spdlog/spdlog.h"
TEST_CASE("PathResolving - config test", "[pathresolving]") {
    {
        for (auto process: {
                 ProcessType::DOCTOR,
                 ProcessType::PATIENT,
                 ProcessType::REGISTRATION,
                 ProcessType::TRIAGE,
                 ProcessType::WAITING_ROOM,
                 ProcessType::TEST
             }) {
            const char *path = getProcessExecPath(process);

            spdlog::info("Path={} ", (path ? path : "NO path found"));

            REQUIRE(path != nullptr);
            REQUIRE(path[0] != '\0');

            int fd = open(path, O_RDONLY);
            REQUIRE(fd != -1);

            char buf;
            ssize_t n = read(fd, &buf, 1);
            REQUIRE(n >= 0);

            close(fd);
        }
    }
}
