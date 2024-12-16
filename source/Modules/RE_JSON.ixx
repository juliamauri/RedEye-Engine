module;

#include <RapidJson/document.h>
#include <string>
#include <unordered_map>

export module JSON;

namespace
{
    struct JsonContainer
    {
        rapidjson::Document document;
        std::string pointer;
    };
    std::unordered_map<uint32_t, JsonContainer> _jsons;
    uint32_t _nextId = 0;
} // namespace

export namespace RE
{
    namespace JSON
    {
        /**
         * @brief Creates a new empty JSON container.
         * @return The ID of the newly created JSON container.
         */
        uint32_t Create()
        {
            uint32_t id = _nextId++;
            _jsons.emplace(id, JsonContainer());
            return id;
        }

        /**
         * @brief Parses a JSON buffer and creates a JSON container.
         * @param buffer The JSON buffer to parse.
         * @param size The size of the JSON buffer.
         * @return The ID of the newly created JSON container.
         */
        uint32_t Parse(const char* buffer, int32_t size)
        {
            uint32_t id = _nextId++;
            auto& jc = _jsons[id];
            jc.document.Parse(buffer, size);
            return id;
        }

        /**
         * @brief Parses a JSON string and creates a JSON container.
         * @param buffer The JSON string to parse.
         * @return The ID of the newly created JSON container.
         */
        uint32_t Parse(const std::string& buffer)
        {
            uint32_t id = _nextId++;
            auto& jc = _jsons[id];
            jc.document.Parse(buffer.c_str());
            return id;
        }

        /**
         * @brief Destroys a JSON container.
         * @param id The ID of the JSON container to destroy.
         */
        void Destroy(const uint32_t id)
        {
            _jsons.erase(id);
        }

        /**
         * @brief Pushes a new name onto the JSON pointer stack.
         * @param id The ID of the JSON container.
         * @param name The name to push onto the pointer stack.
         */
        void Push(const uint32_t id, const char* name)
        {
            _jsons[id].pointer += '/' + name;
        }

        /**
         * @brief Pops the last name from the JSON pointer stack.
         * @param id The ID of the JSON container.
         */
        void Pop(const uint32_t id)
        {
            std::string& pointer = _jsons[id].pointer;
            pointer = pointer.substr(0, pointer.find_last_of('/'));
        }
    } // namespace JSON
} // namespace RE