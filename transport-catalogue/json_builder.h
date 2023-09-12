#pragma once

#include "json.h"

#include <memory>

namespace transport_catalogue {
	namespace detail {
		namespace json {

			class DictContext;
			class ArrayContext;
			class KeyContext;

			class Builder {
			public:

				Node MakeNode(Node::Value value_);
				void AddNode(Node node);

				KeyContext Key(std::string key);
				Builder& Value(Node::Value value);

				DictContext StartDict();
				ArrayContext StartArray();

				Builder& EndDict();
				Builder& EndArray();

				Node Build();

			private:
				Node root_;
				std::vector<std::unique_ptr<Node>> nodes_stack_;
			};

			/*
			* Вспомогательный класс BaseContext из тредов
			* Комментарий наставника:
				От BaseContext мы будем наследоваться class DictValueContext : public BaseContextи т.п
				Метод StartDict() класса BaseContext может возвращать DictItemContext, вызывая StartDict() у Builder-а.
				А в StartDict() у Builder-а будет будет написано в конце return BaseContext{*this}; - что и даст нам ссылку.
			*/
			class BaseContext {
			public:
				BaseContext(Builder& builder);

				KeyContext Key(std::string key);
				DictContext StartDict();
				ArrayContext StartArray();

				Builder& Value(Node::Value value);
				Builder& EndDict();
				Builder& EndArray();

			protected:
				Builder& builder_;
			};

			class KeyContext : public BaseContext {
			public:
				KeyContext(Builder& builder);

				KeyContext Key(std::string key) = delete;

				BaseContext EndDict() = delete;
				BaseContext EndArray() = delete;

				DictContext Value(Node::Value value);
			};

			class DictContext : public BaseContext {
			public:
				DictContext(Builder& builder);

				DictContext StartDict() = delete;

				ArrayContext StartArray() = delete;
				Builder& EndArray() = delete;

				Builder& Value(Node::Value value) = delete;
			};

			class ArrayContext : public BaseContext {
			public:
				ArrayContext(Builder& builder);

				KeyContext Key(std::string key) = delete;

				Builder& EndDict() = delete;

				ArrayContext Value(Node::Value value);
			};
		}
	}
}