/*
 * RedEye Engine - A 3D Game Engine written in C++.
 * Copyright (C) 2018-2024 Julia Mauri and Ruben Sardon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

module;

#include <cstdint>
#include <optional>
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
    std::string pointer = "";
};
std::unordered_map<uint32_t, JsonContainer> _jsons;
uint32_t _nextId = 1;
std::optional<uint32_t> _selected = std::nullopt;

bool _isArray = false;
rapidjson::Value* _array = nullptr;
bool _isArrayObject = false;
rapidjson::Value _arrayObject;
rapidjson::Value* _arrayIter = nullptr;

/**
 * @brief Gets the ID of the JSON container to use.
 * @param id The ID of the JSON container.
 * @return The ID of the JSON container to use.
 */
inline uint32_t GetID(const uint32_t id)
{
    // ASSERT: !_selected.has_value() && id > 0 || _selected.has_value() && id == 0
    return _selected.value_or(id);
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
            _selected.reset();
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
                std::string& pointer = _jsons[GetID(id)].pointer;
                pointer += "/";
                pointer += name;
            }

            /**
             * @brief Pops the last name from the JSON pointer stack.
             * @param id The ID of the JSON container.
             */
            void Pop(const uint32_t id = 0)
            {
                JsonContainer& jc = _jsons[GetID(id)];
                jc.pointer = jc.pointer.substr(0, jc.pointer.find_last_of('/'));
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
                 * @param pushObjects Whether to push objects into the array.
                 * @param id The ID of the JSON container.
                 */
                void Enable(bool pushObjects, const uint32_t id = 0)
                {
                    _isArray = true;
                    JsonContainer& jc = _jsons[GetID(id)];
                    _array = rapidjson::Pointer(jc.pointer.c_str()).Get(jc.document);
                    if (pushObjects)
                    {
                        _isArrayObject = true;
                        _arrayObject = rapidjson::Value();
                        _arrayObject.SetObject();
                    }
                }

                /**
                 * @brief Enables pull mode for the current JSON container.
                 * @param name The name to pull from the JSON container.
                 * @param id The ID of the JSON container.
                 * @return True if the value is an array, false otherwise.
                 */
                bool PullMode(const char* name = nullptr, const uint32_t id = 0)
                {
                    JsonContainer& jc = _jsons[GetID(id)];
                    rapidjson::Value* val = rapidjson::Pointer(GetPointer(jc.pointer, name).c_str()).Get(jc.document);
                    if (val->IsArray() == false)
                    {
                        return false;
                    }
                    _isArray = true;
                    _array = val;
                    return true;
                }

                /**
                 * @brief Disables array mode for the current JSON container.
                 */
                void Disable()
                {
                    _isArray = false;
                    _array = nullptr;
                    _isArrayObject = false;
                    _arrayIter = nullptr;
                    _arrayObject = rapidjson::Value();
                    _arrayObject.SetObject();
                }

                /**
                 * @brief Pushes a new object onto the current JSON array.
                 * @param id The ID of the JSON container.
                 */
                void PushObject(const uint32_t id = 0)
                {
                    if (_isArray && _isArrayObject)
                    {
                        // ASSERT: _isArray == true && _isArrayObject == true
                        // TODO: how to handle value types
                        JsonContainer& jc = _jsons[GetID(id)];
                        _array->PushBack(_arrayObject, jc.document.GetAllocator());
                        _arrayObject = rapidjson::Value();
                        _arrayObject.SetObject();
                    }
                }

                /**
                 * @brief Pulls the next object from the current JSON array.
                 * @param id The ID of the JSON container.
                 * @return True if there are more objects to pull, false otherwise.
                 */
                bool PullObject(const uint32_t id = 0)
                {
                    // ASSERT: _isArray == true
                    if (_isArray)
                    {
                        if (_arrayIter == nullptr)
                        {
                            _arrayIter = _array->Begin();
                        }
                        else
                        {
                            _arrayIter++;
                        }
                        // ASSERT: _arrayIter->IsObject() == true

                        return _arrayIter != _array->End();
                    }
                    return false;
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
                if (_isArray && _isArrayObject)
                {
                    _arrayObject.AddMember(rapidjson::Value().SetString(name, jc.document.GetAllocator()),
                                           rapidjson::Value().SetString(value, jc.document.GetAllocator()),
                                           jc.document.GetAllocator());
                }
                else
                {
                    rapidjson::Pointer(GetPointer(jc.pointer, name).c_str()).Set(jc.document, value);
                }
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
                if (_isArray && _array && _arrayIter)
                {
                    // ASSERT: _arrayIter != _array->End()
                    return _arrayIter->FindMember(name)->value.GetString();
                }
                else
                {
                    JsonContainer& jc = _jsons[GetID(id)];
                    rapidjson::Value* val = rapidjson::Pointer(GetPointer(jc.pointer, name).c_str()).Get(jc.document);
                    return val ? val->GetString() : deflt;
                }
            }

        } // namespace Value
    } // namespace JSON
} // namespace RE
