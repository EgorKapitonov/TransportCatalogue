#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace transport_catalogue {
    namespace detail {
        namespace json {

            class Node;
            // Сохраните объявления Dict и Array без изменения
            using Dict = std::map<std::string, Node>;
            using Array = std::vector<Node>;

            // Эта ошибка должна выбрасываться при ошибках парсинга JSON
            class ParsingError : public std::runtime_error {
            public:
                using runtime_error::runtime_error;
            };

            class Node : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
            public:

                using variant::variant;
                using Value = variant;

                Node() = default;
                Node(std::nullptr_t);
                Node(Array array);
                Node(Dict dict);
                Node(bool value);
                Node(int value);
                Node(double value);
                Node(std::string value);

                const Array& AsArray() const;
                const Dict& AsDict() const;
                bool AsBool() const;
                int AsInt() const;
                double AsDouble() const;
                const std::string& AsString() const;

                // Следующие методы Node сообщают, хранится ли внутри значение некоторого типа:
                bool IsInt() const;
                bool IsDouble() const; // Возвращает true, если в Node хранится int либо double.
                bool IsPureDouble() const; // Возвращает true, если в Node хранится double.
                bool IsBool() const;
                bool IsString() const;
                bool IsNull() const;
                bool IsArray() const;
                bool IsDict() const;

                const Value& GetValue() const {
                    return value_;
                }
            private:
                Value value_;
            };
            // Объекты Node можно сравнивать между собой при помощи == и !=. 
            // Значения равны, если внутри них значения имеют одинаковый тип и содержимое.
            inline bool operator==(const Node& lhs, const Node& rhs) {
                return lhs.GetValue() == rhs.GetValue();
            }
            inline bool operator!=(const Node& lhs, const Node& rhs) {
                return !(lhs == rhs);
            }

            class Document {
            public:
                Document() = default;
                explicit Document(Node root);
                const Node& GetRoot() const;

            private:
                Node root_;
            };

            Document Load(std::istream& input);
            void Print(const Document& doc, std::ostream& output);

            inline bool operator==(const Document& lhs, const Document& rhs) {
                return lhs.GetRoot() == rhs.GetRoot();
            }
            inline bool operator!=(const Document& lhs, const Document& rhs) {
                return !(lhs == rhs);
            }
        }//end namespace json
    }//end namespace detail
}//end namespace transport_catalogue