module;

#include <cstdint>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>

export module JSON;

struct JsonContainer
{
    rapidjson::Document document;
    std::string pointer;
};
static std::unordered_map<uint32_t, JsonContainer> _jsons;
static uint32_t _nextId = 1;
static uint32_t _selected = 0;

static bool _isArray = false;
static rapidjson::Value* _array = nullptr;

/**
 * @brief Gets the ID of the JSON container to use.
 * @param id The ID of the JSON container.
 * @return The ID of the JSON container to use.
 */
inline uint32_t GetID(const uint32_t id)
{
    // ASSERT: _selected == id == 0 || _selected > 0 && id == 0
    uint32_t _options[2] = {id, _selected};
    return _options[_selected];
}

/**
 * @brief Gets the JSON pointer string.
 * @param pointer The current JSON pointer.
 * @param name The name to append to the pointer.
 * @return The updated JSON pointer string.
 */
inline std::string GetPointer(std::string& pointer, const char* name)
{
    return name ? pointer + "/" + name : pointer;
}

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
         * @brief Gets the JSON buffer as a string.
         * @param id The ID of the JSON container.
         * @return The JSON buffer as a string.
         */
        std::string GetBuffer(const uint32_t id = 0)
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            _jsons[GetID(id)].document.Accept(writer);
            return buffer.GetString();
        }

        /**
         * @brief Destroys a JSON container.
         * @param id The ID of the JSON container to destroy.
         */
        void Destroy(const uint32_t id = 0)
        {
            _jsons.erase(GetID(id));
        }

        /**
         * @brief Sets the selected JSON container.
         * @param id The ID of the JSON container to select.
         */
        void PushSelected(const uint32_t id = 0)
        {
            _selected = id;
        }

        /**
         * @brief Deselects the current JSON container.
         */
        void PopSelected()
        {
            _selected = 0;
        }

        namespace Value
        {
            /**
             * @brief Pushes a new name onto the JSON pointer stack.
             * @param name The name to push onto the pointer stack.
             * @param id The ID of the JSON container.
             */
            void Push(const char* name, const uint32_t id = 0)
            {
                _jsons[GetID(id)].pointer += '/' + name;
            }

            /**
             * @brief Pops the last name from the JSON pointer stack.
             * @param id The ID of the JSON container.
             */
            void Pop(const uint32_t id = 0)
            {
                std::string& pointer = _jsons[GetID(id)].pointer;
                pointer = pointer.substr(0, pointer.find_last_of('/'));
            }

            /**
             * @brief Sets the current JSON pointer to an object.
             * @param id The ID of the JSON container.
             */
            void SetObject(const uint32_t id = 0)
            {
                JsonContainer& jc = _jsons[GetID(id)];
                rapidjson::Pointer(jc.pointer.c_str()).Get(jc.document)->SetObject();
            }

            /**
             * @brief Sets the current JSON pointer to an array.
             * @param id The ID of the JSON container.
             */
            void SetArray(const uint32_t id = 0)
            {
                JsonContainer& jc = _jsons[GetID(id)];
                rapidjson::Pointer(jc.pointer.c_str()).Get(jc.document)->SetArray();
            }

            namespace Array
            {
                /**
                 * @brief Enables array mode for the current JSON container.
                 * @param id The ID of the JSON container.
                 */
                void EnableArray(const uint32_t id = 0)
                {
                    _isArray = true;
                    JsonContainer& jc = _jsons[GetID(id)];
                    _array = rapidjson::Pointer(jc.pointer.c_str()).Get(jc.document);
                }

                /**
                 * @brief Disables array mode for the current JSON container.
                 */
                void DisableArray()
                {
                    _isArray = false;
                    _array = nullptr;
                }

                /**
                 * @brief Pushes a new value onto the current JSON array.
                 * @param id The ID of the JSON container.
                 */
                void Push(const uint32_t id = 0)
                {
                    // ASSERT: _isArray == true
                    // TODO: how to handle value types
                    JsonContainer& jc = _jsons[GetID(id)];
                    _array->PushBack(rapidjson::Value(), jc.document.GetAllocator());
                }
            } // namespace Array

            /**
             * @brief Sets a string value at the current JSON pointer.
             * @param value The string value to set.
             * @param name The name to set the value for.
             * @param id The ID of the JSON container.
             */
            void PushString(const char* value, const char* name = nullptr, const uint32_t id = 0)
            {
                JsonContainer& jc = _jsons[GetID(id)];
                rapidjson::Pointer(GetPointer(jc.pointer, name).c_str()).Set(jc.document, value);
            }

            /**
             * @brief Gets a string value from the current JSON pointer.
             * @param name The name to get the value for.
             * @param deflt The default value to return if the value is not found.
             * @param id The ID of the JSON container.
             * @return The string value from the current JSON pointer.
             */
            std::string PullString(const char* name, const char* deflt, const uint32_t id = 0)
            {
                JsonContainer& jc = _jsons[GetID(id)];
                rapidjson::Value* val = rapidjson::Pointer(jc.pointer.c_str()).Get(jc.document);
                return val ? val->GetString() : deflt;
            }

        } // namespace Value
    } // namespace JSON
} // namespace RE
