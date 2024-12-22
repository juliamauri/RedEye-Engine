#include <gtest/gtest.h>
import JSON;

TEST(JsonTest, CreateJsonContainer)
{
    uint32_t id = RE::JSON::Create();
    ASSERT_NE(id, 0);
    RE::JSON::Destroy(id);
}

TEST(JsonTest, ParseJsonString)
{
    const std::string jsonString = R"({"name":"RedEye","version":1})";
    uint32_t id = RE::JSON::Parse(jsonString);
    ASSERT_NE(id, 0);

    std::string buffer = RE::JSON::GetBuffer(id);
    ASSERT_EQ(buffer, jsonString);
    RE::JSON::Destroy(id);
}

TEST(JsonTest, GetJsonValue)
{
    const std::string jsonString = R"({"name":"RedEye","version":1})";
    uint32_t id = RE::JSON::Parse(jsonString);
    ASSERT_NE(id, 0);

    std::string name = RE::JSON::Value::PullString("name", "", id);
    ASSERT_EQ(name, "RedEye");
    RE::JSON::Destroy(id);
}

TEST(JsonTest, SetJsonValue)
{
    uint32_t id = RE::JSON::Create();
    ASSERT_NE(id, 0);

    RE::JSON::Value::PushString("RedEye", "name", id);

    std::string name = RE::JSON::Value::PullString("name", "", id);
    ASSERT_EQ(name, "RedEye");

    RE::JSON::Destroy(id);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}