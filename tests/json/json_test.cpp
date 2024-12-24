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

TEST(JsonTest, ParseEmptyJsonString)
{
    const std::string emptyString = "{}";
    uint32_t id = RE::JSON::Parse(emptyString);
    ASSERT_NE(id, 0);

    std::string buffer = RE::JSON::GetBuffer(id);
    ASSERT_EQ(buffer, emptyString);
    RE::JSON::Destroy(id);
}

// TODO: Handle invalid JSON strings
// TEST(JsonTest, ParseInvalidJsonString)
// {
//     const std::string invalidString = "{invalid}";
//     uint32_t id = RE::JSON::Parse(invalidString);
//     ASSERT_EQ(id, 0); // Expecting id to be 0 or equivalent failure state
// }

TEST(JsonTest, GetNonExistentJsonValue)
{
    const std::string jsonString = R"({"name":"RedEye"})";
    uint32_t id = RE::JSON::Parse(jsonString);
    ASSERT_NE(id, 0);

    std::string version = RE::JSON::Value::PullString("version", "default", id);
    ASSERT_EQ(version, "default");
    RE::JSON::Destroy(id);
}

TEST(JsonTest, HandleLargeJson)
{
    std::string largeJson = R"({"data":[)";
    for (int i = 0; i < 1000; ++i)
    {
        largeJson += std::to_string(i) + ",";
    }
    largeJson.back() = ']';
    largeJson += "}";

    uint32_t id = RE::JSON::Parse(largeJson);
    ASSERT_NE(id, 0);
    RE::JSON::Destroy(id);
}

TEST(JsonTest, DestroyJsonContainer)
{
    uint32_t id = RE::JSON::Create();
    ASSERT_NE(id, 0);

    RE::JSON::Destroy(id);

    std::string name = RE::JSON::Value::PullString("name", "", id);
    ASSERT_EQ(name, "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
