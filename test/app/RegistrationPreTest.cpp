#include "catch2/catch_test_macros.hpp"
#include "MessageQueue.hpp"
#include "ProcessManager.hpp"
#include "childProcesses/Registration.hpp"

#include <cstring>
#include <memory>
#include <unistd.h>

#include "SemaphoreArray.hpp"

namespace {
    Patient::BasicData mockPatient(int socialId, bool isVIP, bool diesNow) {
        Patient::BasicData data{};
        data.socialId = socialId;
        data.isVIP = isVIP;
        std::strncpy(data.name, "Nazywam sie: <imie> <nazwisko>", sizeof(data.name) - 1);
        std::strncpy(data.address, "Jakiś adres", sizeof(data.address) - 1);
        data.phone = static_cast<short>(100 + (socialId % 10000));
        data.ill = Patient::INFECTION;
        data.diesNow = diesNow;
        return data;
    }
}

TEST_CASE("Registration prioritizes VIP messages", "[registration]") {
    ProcessManager pm;
    REQUIRE(pm.assignProcess(std::make_unique<Registration>(20)));

    MessageQueue windowIn(Registration::QID_WINDOW_1_IN, false);
    MessageQueue windowOut(Registration::QID_WINDOW_1_OUT, false);

    const auto normal = mockPatient(101, false, false);
    const auto vip = mockPatient(202, true, false);

    REQUIRE(windowIn.send(normal, Registration::QTYPE_WINDOW_IN) == 0);
    REQUIRE(windowIn.send(vip, Registration::QTYPE_WINDOW_IN_VIP) == 0);

    Registration::Q_WINDOW_OUT_STRUCT response{};
    REQUIRE(windowOut.receive(response, 0, true) > 0);
    REQUIRE(response.youCanHurry == true);

    REQUIRE(windowOut.receive(response, 0, true) > 0);
    REQUIRE(response.youCanHurry == false);
}

TEST_CASE("Registration window returns hurry for VIP or diesNow", "[registration]") {
    ProcessManager pm;
    REQUIRE(pm.assignProcess(std::make_unique<Registration>(20)));


    MessageQueue windowIn(Registration::QID_WINDOW_1_IN, false);
    MessageQueue windowOut(Registration::QID_WINDOW_1_OUT, false);

    const auto diesNow = mockPatient(303, false, true);
    const auto vip = mockPatient(404, true, false);

    REQUIRE(windowIn.send(diesNow, Registration::QTYPE_WINDOW_IN) == 0);
    REQUIRE(windowIn.send(vip, Registration::QTYPE_WINDOW_IN_VIP) == 0);

    Registration::Q_WINDOW_OUT_STRUCT response{};
    REQUIRE(windowOut.receive(response, diesNow.socialId,true) > 0);
    REQUIRE(response.youCanHurry == true);

    REQUIRE(windowOut.receive(response, vip.socialId, true) > 0);
    REQUIRE(response.youCanHurry == true);

}
