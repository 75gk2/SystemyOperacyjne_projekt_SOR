#include "DataSOR.h"
#include "catch2/catch_test_macros.hpp"
#include "MessageQueue.hpp"
#include "utils.hpp"


TEST_CASE("MessageQueue creator and client", "[messagequeue]") {
    {
        MessageQueue mq_creator('B', true);
        REQUIRE(mq_creator.getMsgId() != -1);


        MessageQueue mq_user('B', false);
        REQUIRE(mq_user.getMsgId() == mq_creator.getMsgId());
    }
}

TEST_CASE("MessageQueue send and receive struct message", "[messagequeue]") {
    MessageQueue mq('C', true);

    auto data = DataSOR();
    data.isOpen = true;
    data.lekarzePIDs[0] = 5678;

    int result = mq.send<DataSOR>(data, 1);
    REQUIRE(result == 0);

    DataSOR received_data;
    result = mq.receive<DataSOR>(received_data, 1);
    REQUIRE(result > 0);
    REQUIRE(received_data.isOpen == true);
    REQUIRE(received_data.lekarzePIDs[0] == 5678);
}


TEST_CASE("MessageQueue send and receive in reverse order by mtype", "[messagequeue]") {
    MessageQueue mq('A', true);

    int msg1 = 100;
    mq.send<int>(msg1, 1);

    int msg2 = 200;
    mq.send<int>(msg2, 2);

    int received = 0;
    int result = mq.receive<int>(received, 2);

    REQUIRE(result > 0);
    REQUIRE(received == msg2);
    
    result = mq.receive<int>(received, 1);
    REQUIRE(result > 0);
    REQUIRE(received == msg1);
}
