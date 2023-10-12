#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <charconv>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression): ast_(ParseFormulaAST(expression)) {}
    //using Value = std::variant<double, FormulaError>;
    Value Evaluate(const SheetInterface& sheet) const override {

        std::function<double(Position)> lambda = [&sheet](Position pos) {

            if (!pos.IsValid()) {
                throw InvalidPositionException("invalid position");
            }

            double result = 0.0;
            auto cell = sheet.GetCell(pos);
            if (!cell) {
                return result;
            }
            auto value = cell->GetValue();
            

            if (std::holds_alternative<double>(value)) {
                result = std::get<double>(value);
            }
            else if(std::holds_alternative<std::string>(value)){
                std::string text = std::get<std::string>(value);
                if (text.empty()) {
                    return result;
                }

                for (const auto& lit : text) {
                    if (!std::isdigit(lit)) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }

                result = std::stod(text);

                /*
                Тренажер не пропускает такую конструкцию -_-
                const std::from_chars_result val = std::from_chars(text.data(), text.data() + text.size(), result);

                if (val.ec != std::errc() || val.ptr != text.data() + text.size()) {
                    throw FormulaError(FormulaError::Category::Value);
                }
                */
                

            }
            else {
                throw std::get<FormulaError>(value);
            }

            return result;
   
        };

        Value result;

        try {
            result = ast_.Execute(lambda);
            return result;
        }
        catch(const FormulaError& error){
             result = error;
             return result;
        }
    }

    std::string GetExpression() const override {
        std::stringstream sout;
        ast_.PrintFormula(sout);

        return sout.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::set<Position> buf;
        for (const auto& cell : ast_.GetCells()) {
            buf.insert(cell);
        }

        return std::vector<Position>(buf.begin(), buf.end());
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}