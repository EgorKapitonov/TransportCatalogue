#include "json_builder.h"

namespace transport_catalogue {
	namespace detail {
		namespace json {
			/*--- BaseContext ---*/
			BaseContext::BaseContext(Builder& builder)
				: builder_(builder) {}

			KeyContext BaseContext::Key(std::string key) {
				return builder_.Key(key);
			}
			DictContext BaseContext::StartDict() {
				return DictContext(builder_.StartDict());
			}
			ArrayContext BaseContext::StartArray() {
				return ArrayContext(builder_.StartArray());
			}

			Builder& BaseContext::Value(Node::Value value) {
				return builder_.Value(value);
			}
			Builder& BaseContext::EndDict() {
				return builder_.EndDict();
			}
			Builder& BaseContext::EndArray() {
				return builder_.EndArray();
			}

			/*--- KeyContext ---*/

			KeyContext::KeyContext(Builder& builder)
				: BaseContext(builder) {}

			DictContext KeyContext::Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}

			/*--- DictContext ---*/
			DictContext::DictContext(Builder& builder)
				: BaseContext(builder) {}
			/*--- ArrayContext ---*/
			ArrayContext::ArrayContext(Builder& builder)
				: BaseContext(builder) {}

			ArrayContext ArrayContext::Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}
			/*--- Builder ---*/
			Node Builder::MakeNode(Node::Value value) {
				Node node;

				if (std::holds_alternative<bool>(value)) {
					bool get_bool = std::get<bool>(value);
					node = Node(get_bool);
				}
				else if (std::holds_alternative<int>(value)) {
					int get_int = std::get<int>(value);
					node = Node(get_int);
				}
				else if (std::holds_alternative<double>(value)) {
					double get_double = std::get<double>(value);
					node = Node(get_double);
				}
				else if (std::holds_alternative<std::string>(value)) {
					std::string get_str = std::get<std::string>(value);
					node = Node(std::move(get_str));
				}
				else if (std::holds_alternative<Array>(value)) {
					Array get_array = std::get<Array>(value);
					node = Node(std::move(get_array));
				}
				else if (std::holds_alternative<Dict>(value)) {
					Dict get_dict = std::get<Dict>(value);
					node = Node(std::move(get_dict));
				}
				else {
					node = Node();
				}
				return node;
			}
			void Builder::AddNode(Node node) {
				if (nodes_stack_.empty()) {
					if (!root_.IsNull()) {
						throw std::logic_error("root has been AddNode");
					}
					root_ = node;
					return;
				}
				else {
					if (!nodes_stack_.back()->IsArray()
						&& !nodes_stack_.back()->IsString()) {
						throw std::logic_error("unable to create Node");
					}
					if (nodes_stack_.back()->IsArray()) {
						Array arr = nodes_stack_.back()->AsArray();
						arr.emplace_back(node);

						nodes_stack_.pop_back();
						auto arr_ptr = std::make_unique<Node>(arr);
						nodes_stack_.emplace_back(std::move(arr_ptr));

						return;
					}
					if (nodes_stack_.back()->IsString()) {
						std::string str = nodes_stack_.back()->AsString();
						nodes_stack_.pop_back();
						if (nodes_stack_.back()->IsDict()) {
							Dict dictionary = nodes_stack_.back()->AsDict();
							dictionary.emplace(std::move(str), node);

							nodes_stack_.pop_back();
							auto dictionary_ptr = std::make_unique<Node>(dictionary);
							nodes_stack_.emplace_back(std::move(dictionary_ptr));
						}
						return;
					}
				}
			}

			KeyContext Builder::Key(std::string key) {
				if (nodes_stack_.empty()) {
					throw std::logic_error("error when creating Key");
				}
				auto key_ptr = std::make_unique<Node>(key);
				if (nodes_stack_.back()->IsDict()) {
					nodes_stack_.emplace_back(std::move(key_ptr));
				}
				return KeyContext(*this);
			}
			Builder& Builder::Value(Node::Value value) {
				AddNode(MakeNode(value));
				return *this;
			}

			DictContext Builder::StartDict() {
				nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
				return DictContext(*this);
			}
			ArrayContext Builder::StartArray() {
				nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
				return ArrayContext(*this);
			}

			Builder& Builder::EndDict() {
				if (nodes_stack_.empty()) {
					throw std::logic_error("error when creating EndDict");
				}
				Node node = *nodes_stack_.back();
				if (!node.IsDict()) {
					throw std::logic_error("object isn't dictionary");
				}
				nodes_stack_.pop_back();
				AddNode(node);
				return *this;
			}
			Builder& Builder::EndArray() {
				if (nodes_stack_.empty()) {
					throw std::logic_error("unable to close without opening");
				}
				Node node = *nodes_stack_.back();
				if (!node.IsArray()) {
					throw std::logic_error("object isn't array");
				}
				nodes_stack_.pop_back();
				AddNode(node);
				return *this;
			}

			Node Builder::Build() {
				if (root_.IsNull()) {
					throw std::logic_error("empty json");
				}
				else if (!nodes_stack_.empty()) {
					throw std::logic_error("invalid json");
				}
				return root_;
			}
		}
	}
}