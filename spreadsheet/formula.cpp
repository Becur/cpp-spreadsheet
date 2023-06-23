#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <functional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
    : ast_(ParseFormulaAST(expression)){}
    Value Evaluate(const SheetInterface& sheet) const override {
        std::function<Value(const Position)> func = [&sheet](const Position pos_) -> Value {
            auto ref = sheet.GetCell(pos_);
            if(ref == nullptr){
                return 0.0;
            }
            auto val = ref->GetValue();
            if(std::holds_alternative<double>(val)){
                return std::get<double>(val);
            }
            else if(std::holds_alternative<FormulaError>(val)){
                return std::get<FormulaError>(val);
            }
            else{
                std::string text = std::move(std::get<std::string>(val));
                if(text.empty()){
                    return 0.0;
                }
                try{
                    size_t count_char = 0;
                    double res = std::stod(std::move(text), &count_char);
                    if(count_char == text.size()){
                        return res;
                    }
                    else{
                        return FormulaError(FormulaError::Category::Value);
                    }
                }
                catch(...){
                    return FormulaError(FormulaError::Category::Value);
                }
            }
        };
        try{
            return ast_.Execute(func);
        }
        catch(FormulaError error){
            return error;
        }
    }
    std::string GetExpression() const override{
        std::stringstream res(std::ios::out);
        ast_.PrintFormula(res);
        return res.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::forward_list<Position> list = ast_.GetCells();
        std::vector<Position> res(std::make_move_iterator(list.begin()), std::make_move_iterator(list.end()));
        std::sort(res.begin(), res.end());
        res.erase(std::unique(res.begin(), res.end()), res.end());
        return res;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try{
        return std::make_unique<Formula>(std::move(expression));
    }
    catch(...){
        throw FormulaException("Incorrect formula");
    }
}