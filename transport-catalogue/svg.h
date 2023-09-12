#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red(red)
        , green(green)
        , blue(blue) {
        };
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};
inline void PrintColor(std::ostream& out, Rgb& rgb);
    
class Rgba {
public:
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : red(red)
        , green(green)
        , blue(blue)
        , opacity(opacity) {
        };
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};
inline void PrintColor(std::ostream& out, Rgba& rgba);
 
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

inline void PrintColor(std::ostream& out, std::monostate);
inline void PrintColor(std::ostream& out, std::string& color);
std::ostream& operator<<(std::ostream& out, const Color& color);
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineCap stroke_line_cap) {
    using namespace std::literals;
    if (stroke_line_cap == StrokeLineCap::BUTT) {
        out << "butt"sv;
    } else if (stroke_line_cap == StrokeLineCap::ROUND) {
        out << "round"sv;
    } else if (stroke_line_cap == StrokeLineCap::SQUARE) {
        out << "square"sv;
    }
    return out;
}
    
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join) {
    using namespace std::literals;
    if (line_join == StrokeLineJoin::ARCS) {
        out << "arcs"sv;
    } else if (line_join == StrokeLineJoin::BEVEL) {
        out << "bevel"sv;
    } else if (line_join == StrokeLineJoin::MITER) {
        out << "miter"sv;
    } else if (line_join == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip"sv;
    } else if (line_join == StrokeLineJoin::ROUND) {
        out << "round"sv;
    }
    return out;
}
    
template <typename Path>
class PathProps {
public: 
    Path& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Path& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Path& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Path& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }
    Path& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;
    
    void RenderAttrs(std::ostream &out) const {
        using namespace std::literals;
 
        if (fill_color_ != std::nullopt) {
            out << "fill=\""sv << fill_color_.value() << "\" "sv;
        }
        if (stroke_color_ != std::nullopt) {
            out << "stroke=\""sv << stroke_color_.value() << "\" "sv;
        }
        if (stroke_width_ != std::nullopt) {
            out << "stroke-width=\""sv << stroke_width_.value() << "\" "sv;
        }
        if (line_cap_ != std::nullopt) {
            out << "stroke-linecap=\""sv << line_cap_.value() << "\" "sv;
        }
        if (line_join_ != std::nullopt) {
            out << "stroke-linejoin=\""sv << line_join_.value() << "\" "sv;
        }
    }
private:
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
    
    // Для поддержания method chaining, то есть возвращать ссылку на тип объекта, у которого были вызваны, по рекомедации наставника использовать функцию
    Path& AsOwner() {
        return static_cast<Path&>(*this);
    }
};
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;
    /*
        Описывается текстовым содержимым, 
        координатами опорной точки, 
        смещением относительно опорной точки, 
        размером 
        и названием шрифта.
    */
    
    Point reference_point_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
    
    static std::string delete_spaces(const std::string& str);
    static std::string uniq_symbols(const std::string& str);
};
        
class ObjectContainer {
public:
    template<typename Obj>
    void Add(Obj obj);
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    virtual ~ObjectContainer() = default;
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
template <typename Obj>
void ObjectContainer::Add(Obj obj) {
    objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}
class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};    
class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object> &&obj) override {
        objects_.emplace_back(std::move(obj));
    }
    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};
}  // namespace svg