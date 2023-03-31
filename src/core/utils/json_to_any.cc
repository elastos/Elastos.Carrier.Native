/*
 * Copyright (c) 2022 - 2023 trinity-tech.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "json_to_any.h"

std::any jsonToAny(const nlohmann::json& j)
{
    std::any a;

    if (j.is_object()) {
        a = jsonToMap(j);
    } else if (j.is_array()) {
        a = jsonToVector(j);
    } else if (j.is_string()) {
        a = j.get<std::string>();
    } else if (j.is_number_integer()) {
        a = j.get<int64_t>();
    } else if (j.is_number_unsigned()) {
        a = j.get<uint64_t>();
    } else if (j.is_number_float()) {
        a = j.get<double>();
    } else if (j.is_boolean()) {
        a = j.get<bool>();
    } else if (j.is_null()) {
        a = nullptr;
    }

    return a;
}

std::vector<std::any> jsonToVector(const nlohmann::json& j)
{
    std::vector<std::any> v;
    v.reserve(j.size());

    for (auto it = j.cbegin(); it != j.cend(); ++it)
        v.push_back(jsonToAny(it.value()));

    return v;
}

std::map<std::string, std::any> jsonToMap(const nlohmann::json& j)
{
    std::map<std::string, std::any> m;

    for (auto it = j.cbegin(); it != j.cend(); ++it)
        m[it.key()] = jsonToAny(it.value());

    return m;
}

std::ostream& operator <<(std::ostream&o, std::map<std::string, std::any> m)
{
    o << "{" << std::endl;

    for (auto it = m.cbegin(); it != m.cend(); ++it)
        o << it->first << ": " << it->second << std::endl;

    o << "}" << std::endl;

    return o;
}

std::ostream& operator <<(std::ostream&o, std::vector<std::any> v)
{
    o << "[" << std::endl;

    for (auto it = v.cbegin(); it != v.cend(); ++it)
        o << *it << std::endl;

    o << "]" << std::endl;

    return o;
}

std::ostream& operator <<(std::ostream& o, std::any a)
{
    if (a.type() == typeid(int64_t))
        o << std::any_cast<int64_t>(a);
    else if (a.type() == typeid(uint64_t))
        o << std::any_cast<uint64_t>(a);
    else if (a.type() == typeid(double))
        o << std::any_cast<double>(a);
    else if (a.type() == typeid(bool))
        o << std::any_cast<bool>(a);
    else if (a.type() == typeid(std::string))
        o << std::any_cast<std::string>(a);
    else if (a.type() == typeid(std::map<std::string, std::any>))
        o << std::any_cast<std::map<std::string, std::any>>(a);
    else if (a.type() == typeid(std::vector<std::any>))
        o << std::any_cast<std::vector<std::any>>(a);
    else
        o << "N/A:" << a.type().name();

    return o;
}
