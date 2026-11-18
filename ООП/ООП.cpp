#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <clocale>


class Vector2D {
public:
    const int dx;
    const int dy;

    constexpr Vector2D(int dx, int dy) noexcept : dx(dx), dy(dy) {}

    constexpr Vector2D operator+(const Vector2D& other) const noexcept {
        return Vector2D(dx + other.dx, dy + other.dy);
    }

    constexpr Vector2D operator-(const Vector2D& other) const noexcept {
        return Vector2D(dx - other.dx, dy - other.dy);
    }

    constexpr bool operator==(const Vector2D& other) const noexcept {
        return dx == other.dx && dy == other.dy;
    }

    constexpr bool operator!=(const Vector2D& other) const noexcept {
        return !(*this == other);
    }
};

class Point2D {
public:
    const int x;
    const int y;

    constexpr Point2D(int x, int y) noexcept : x(x), y(y) {}

    constexpr Point2D operator+(const Vector2D& vec) const noexcept {
        return Point2D(x + vec.dx, y + vec.dy);
    }

    constexpr Point2D operator-(const Vector2D& vec) const noexcept {
        return Point2D(x - vec.dx, y - vec.dy);
    }

    constexpr Vector2D operator-(const Point2D& other) const noexcept {
        return Vector2D(x - other.x, y - other.y);
    }

    constexpr bool operator==(const Point2D& other) const noexcept {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Point2D& other) const noexcept {
        return !(*this == other);
    }
};

class Rect {
public:
    const Point2D topLeft;
    const int width;
    const int height;

    Rect(Point2D topLeft, int width, int height)
        : topLeft(topLeft), width(width), height(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Ширина и высота прямоугольника должны быть положительными");
        }
    }

    constexpr int left() const noexcept { return topLeft.x; }
    constexpr int top() const noexcept { return topLeft.y; }
    constexpr int right() const noexcept { return topLeft.x + width - 1; }
    constexpr int bottom() const noexcept { return topLeft.y + height - 1; }

    constexpr bool contains(const Point2D& p) const noexcept {
        return p.x >= left() && p.x <= right() &&
            p.y >= top() && p.y <= bottom();
    }

    constexpr bool operator==(const Rect& other) const noexcept {
        return topLeft == other.topLeft &&
            width == other.width &&
            height == other.height;
    }

    constexpr bool operator!=(const Rect& other) const noexcept {
        return !(*this == other);
    }
};


class UIElement {
private:
    Rect bounds_;
    bool enabled_;

protected:
    UIElement(Rect bounds, bool enabled = true)
        : bounds_(bounds), enabled_(enabled) {
    }

public:
    virtual ~UIElement() = default;

    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;

    const Rect& bounds() const noexcept { return bounds_; }

    bool contains(const Point2D& point) const noexcept {
        return bounds_.contains(point);
    }

    bool isEnabled() const noexcept { return enabled_; }

    void setEnabled(bool value) noexcept {
        enabled_ = value;
        onEnabledChanged(value);
    }

    bool click() {
        if (!enabled_) {
            return false;
        }
        onClick();
        return true;
    }

    virtual bool canReceiveFocus() const noexcept {
        return false;
    }

    virtual void handleTextInput(const std::string& /*text*/) {}

    virtual std::string toString() const = 0;

    virtual void draw(std::vector<std::string>& canvas, bool isFocused) const {
        const int x0 = bounds_.left();
        const int y0 = bounds_.top();
        const int w = bounds_.width;
        const int h = bounds_.height;

        const char borderH = isFocused ? '#' : '-';
        const char borderV = isFocused ? '#' : '|';
        const char corner = isFocused ? '#' : '+';

        for (int dy = 0; dy < h; ++dy) {
            int cy = y0 + dy;
            if (cy < 0 || cy >= static_cast<int>(canvas.size())) continue;

            for (int dx = 0; dx < w; ++dx) {
                int cx = x0 + dx;
                if (cx < 0 || cx >= static_cast<int>(canvas[cy].size())) continue;

                bool isTopOrBottom = (dy == 0 || dy == h - 1);
                bool isLeftOrRight = (dx == 0 || dx == w - 1);

                if (isTopOrBottom && isLeftOrRight) {
                    canvas[cy][cx] = corner;
                }
                else if (isTopOrBottom) {
                    canvas[cy][cx] = borderH;
                }
                else if (isLeftOrRight) {
                    canvas[cy][cx] = borderV;
                }
                else {
                    canvas[cy][cx] = ' ';
                }
            }
        }

        if (h >= 3 && w >= 4) {
            std::string label = innerLabel();
            int maxLen = w - 2;
            if (static_cast<int>(label.size()) > maxLen) {
                label = label.substr(0, maxLen);
            }
            int textY = y0 + h / 2;
            if (textY >= 0 && textY < static_cast<int>(canvas.size())) {
                for (size_t i = 0; i < label.size(); ++i) {
                    int textX = x0 + 1 + static_cast<int>(i);
                    if (textX >= 0 && textX < static_cast<int>(canvas[textY].size())) {
                        canvas[textY][textX] = label[i];
                    }
                }
            }
        }
    }

protected:
    virtual void onClick() = 0;
    virtual void onEnabledChanged(bool /*enabled*/) noexcept {}
    virtual std::string innerLabel() const = 0;
};

class Button : public UIElement {
private:
    std::string caption_;
    int clickCount_ = 0;

public:
    Button(Point2D pos, int width, int height, std::string caption, bool enabled = true)
        : UIElement(Rect(pos, width, height), enabled), caption_(std::move(caption)) {
    }

    int clickCount() const noexcept { return clickCount_; }
    const std::string& caption() const noexcept { return caption_; }

    std::string toString() const override {
        std::ostringstream oss;
        oss << caption_ << ":" << clickCount_
            << (isEnabled() ? "" : " [отключено]");
        return oss.str();
    }

protected:
    void onClick() override {
        ++clickCount_;
    }

    std::string innerLabel() const override {
        return caption_ + ":" + std::to_string(clickCount_);
    }
};

class CheckBox : public UIElement {
private:
    std::string label_;
    bool checked_;

public:
    CheckBox(Point2D pos, int width, int height, std::string label,
        bool initialChecked = false, bool enabled = true)
        : UIElement(Rect(pos, width, height), enabled),
        label_(std::move(label)),
        checked_(initialChecked) {
    }

    bool isChecked() const noexcept { return checked_; }
    const std::string& label() const noexcept { return label_; }

    std::string toString() const override {
        std::ostringstream oss;
        oss << (checked_ ? "[x] " : "[ ] ") << label_
            << (isEnabled() ? "" : " [отключено]");
        return oss.str();
    }

protected:
    void onClick() override {
        checked_ = !checked_;
    }

    std::string innerLabel() const override {
        return std::string(checked_ ? "[x] " : "[ ] ") + label_;
    }
};


class TextField : public UIElement {
private:
    std::string placeholder_;
    std::string text_;

public:
    TextField(Point2D pos, int width, int height, std::string placeholder,
        std::string initialText = "", bool enabled = true)
        : UIElement(Rect(pos, width, height), enabled),
        placeholder_(std::move(placeholder)),
        text_(std::move(initialText)) {
    }

    const std::string& text() const noexcept { return text_; }
    const std::string& placeholder() const noexcept { return placeholder_; }

    bool canReceiveFocus() const noexcept override {
        return isEnabled();
    }

    void handleTextInput(const std::string& input) override {
        if (!isEnabled()) {
            return;
        }
        text_ += input;
    }

    std::string toString() const override {
        std::ostringstream oss;
        oss << "ТекстовоеПоле(" << placeholder_ << "): \"" << text_ << "\""
            << (isEnabled() ? "" : " [отключено]");
        return oss.str();
    }

protected:
    void onClick() override {}

    std::string innerLabel() const override {
        return text_.empty() ? placeholder_ : text_;
    }
};


class Form {
private:
    int width_;
    int height_;
    std::vector<std::unique_ptr<UIElement>> elements_;
    std::optional<size_t> focusedIndex_;

    std::optional<size_t> findTopElementIndex(const Point2D& point, bool onlyEnabled) const noexcept {
        for (size_t i = elements_.size(); i > 0; --i) {
            size_t idx = i - 1;
            if (elements_[idx]->contains(point)) {
                if (!onlyEnabled || elements_[idx]->isEnabled()) {
                    return idx;
                }
            }
        }
        return std::nullopt;
    }

public:
    Form(int width, int height) : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Размеры формы должны быть положительными");
        }
    }

    template <typename T, typename... Args>
    T* addElement(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = ptr.get();
        elements_.push_back(std::move(ptr));
        return rawPtr;
    }

    bool removeElement(const UIElement* element) {
        for (size_t i = 0; i < elements_.size(); ++i) {
            if (elements_[i].get() == element) {
                elements_.erase(elements_.begin() + static_cast<ptrdiff_t>(i));

                if (focusedIndex_.has_value()) {
                    if (*focusedIndex_ == i) {
                        focusedIndex_.reset();
                    }
                    else if (*focusedIndex_ > i) {
                        --(*focusedIndex_);
                    }
                }
                return true;
            }
        }
        return false;
    }

    void setElementEnabled(UIElement* element, bool enabled) {
        if (!element) return;
        element->setEnabled(enabled);

        if (!enabled && focusedIndex_.has_value()) {
            if (elements_[*focusedIndex_].get() == element) {
                focusedIndex_.reset();
            }
        }
    }

    UIElement* elementAt(const Point2D& point) const noexcept {
        auto idx = findTopElementIndex(point, /*onlyEnabled=*/false);
        return idx.has_value() ? elements_[*idx].get() : nullptr;
    }

    bool clickAt(const Point2D& point) {
        auto idx = findTopElementIndex(point, /*onlyEnabled=*/true);
        if (!idx.has_value()) {
            focusedIndex_.reset();
            return false;
        }

        UIElement* target = elements_[*idx].get();
        bool handled = target->click();

        if (handled) {
            if (target->canReceiveFocus()) {
                focusedIndex_ = *idx;
            }
            else {
                focusedIndex_.reset();
            }
        }
        return handled;
    }

    void sendTextInput(const std::string& text) {
        if (!focusedIndex_.has_value()) {
            return;
        }
        UIElement* focused = elements_[*focusedIndex_].get();
        if (focused->isEnabled() && focused->canReceiveFocus()) {
            focused->handleTextInput(text);
        }
    }

    bool focusNext() {
        if (elements_.empty()) {
            focusedIndex_.reset();
            return false;
        }

        size_t count = elements_.size();
        size_t startIdx = focusedIndex_.has_value() ? (*focusedIndex_ + 1) % count : 0;

        for (size_t offset = 0; offset < count; ++offset) {
            size_t candidate = (startIdx + offset) % count;
            if (elements_[candidate]->isEnabled() && elements_[candidate]->canReceiveFocus()) {
                focusedIndex_ = candidate;
                return true;
            }
        }

        focusedIndex_.reset();
        return false;
    }

    UIElement* focusedElement() const noexcept {
        return focusedIndex_.has_value() ? elements_[*focusedIndex_].get() : nullptr;
    }

    void render(std::ostream& out) const {
        std::vector<std::string> canvas(height_, std::string(width_, ' '));

        for (size_t i = 0; i < elements_.size(); ++i) {
            bool isFocused = (focusedIndex_.has_value() && *focusedIndex_ == i);
            elements_[i]->draw(canvas, isFocused);
        }

        out << "+" << std::string(width_, '-') << "+\n";
        for (const auto& row : canvas) {
            out << "|" << row << "|\n";
        }
        out << "+" << std::string(width_, '-') << "+\n";
    }
};



void runTests() {
    Point2D p1(10, 20);
    Vector2D v1(5, -3);
    Point2D p2 = p1 + v1;
    Vector2D v2 = p2 - p1;
    assert(v2 == v1);
    assert(p2 == Point2D(15, 17));
    assert(p1 == Point2D(10, 20));

    Form form(40, 20);
    auto* btn = form.addElement<Button>(Point2D(1, 1), 14, 3, "Отправить");
    auto* chk = form.addElement<CheckBox>(Point2D(1, 5), 16, 3, "Согласен", false);
    auto* txt = form.addElement<TextField>(Point2D(1, 9), 20, 3, "Имя");

    assert(btn->clickCount() == 0);
    assert(form.clickAt(Point2D(3, 2)));
    assert(btn->clickCount() == 1);

    assert(!chk->isChecked());
    assert(form.clickAt(Point2D(3, 6)));
    assert(chk->isChecked());

    assert(form.clickAt(Point2D(5, 10)));
    assert(form.focusedElement() == txt);
    form.sendTextInput("Иван");
    assert(txt->text() == "Иван");
}

int main() {
    std::setlocale(LC_ALL, "Russian");

    runTests();
    std::cout << "Все автоматические тесты успешно пройдены!\n\n";
    std::cout << "========================================\n";
    std::cout << " ИНТЕРАКТИВНЫЙ РЕЖИМ ФОРМЫ (для проверки)\n";
    std::cout << "========================================\n\n";

 
    Form form(35, 15);
    auto* btn = form.addElement<Button>(Point2D(2, 1), 16, 3, "Кнопка");
    auto* chk = form.addElement<CheckBox>(Point2D(2, 5), 18, 3, "Флажок");
    auto* txt = form.addElement<TextField>(Point2D(2, 9), 26, 3, "ПолеВвода");

    char command = '\0';
    while (true) {
        std::cout << "\nТекущее состояние формы:\n";
        form.render(std::cout);

        std::cout << "\nСостояние элементов:\n";
        std::cout << "  1. " << btn->toString() << "\n";
        std::cout << "  2. " << chk->toString() << "\n";
        std::cout << "  3. " << txt->toString() << "\n";
        if (form.focusedElement() != nullptr) {
            std::cout << "  [Фокус активен на одном из элементов]\n";
        }
        else {     
            std::cout << "  [Фокус отсутствует]\n";
        }

        std::cout << "\nМеню действий:\n";
        std::cout << "  [c] Кликнуть по координатам (X Y, например: 4 2)\n";
        std::cout << "  [t] Переключить фокус (Tab)\n";
        std::cout << "  [w] Ввести текст в элемент с фокусом\n";
        std::cout << "  [q] Выход из программы\n";
        std::cout << "Выберите действие: ";

        std::cin >> command;
        if (command == 'q' || command == 'Q') {
            break;
        }
        else if (command == 'c' || command == 'C') {
            int x, y;
            std::cout << "Введите координаты X и Y через пробел: ";
            std::cin >> x >> y;
            bool res = form.clickAt(Point2D(x, y));
            std::cout << (res ? "-> Клик успешно обработан!" : "-> Клик мимо элементов или по отключенному элементу.") << "\n";
        }
        else if (command == 't' || command == 'T') {
            form.focusNext();
            std::cout << "-> Фокус переключен на следующий элемент.\n";
        }
        else if (command == 'w' || command == 'W') {
            std::string inputStr;
            std::cout << "Введите текст для отправки: ";
            std::cin >> inputStr;
            form.sendTextInput(inputStr);
            std::cout << "-> Текст отправлен.\n";
        }
    }

    return 0;
}