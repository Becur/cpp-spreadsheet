#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position is not valid"s);
    }
    Cell new_cell(this, pos);
    new_cell.Set(text);
    sheet_[pos] = std::move(new_cell);
    printable_size.cols = std::max(pos.col + 1, printable_size.cols);
    printable_size.rows = std::max(pos.row + 1, printable_size.rows);
    ++count_rows[pos.row + 1];
    ++count_cols[pos.col + 1];
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position is not valid"s);
    }
    return sheet_.count(pos) ? &sheet_.at(pos) : nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position is not valid"s);
    }
    return sheet_.count(pos) ? &sheet_.at(pos) : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position is not valid"s);
    }
    if(sheet_.count(pos)){
        sheet_.erase(pos);
        --count_cols[pos.col + 1];
        if(pos.col + 1 == printable_size.cols){
            size_t new_size_cols = 0;
            for(size_t i = pos.col + 1; i > 0; --i){
                if(count_cols.count(i) && (count_cols[i] > 0)){
                    new_size_cols = i;
                    break;
                }
            }
            printable_size.cols = new_size_cols;
        }
        --count_rows[pos.row + 1];
        if(pos.row + 1 == printable_size.rows){
            size_t new_size_rows = 0;
            for(size_t i = pos.row + 1; i > 0; --i){
                if(count_rows.count(i) && (count_rows[i] > 0)){
                    new_size_rows = i;
                    break;
                }
            }
            printable_size.rows = new_size_rows;
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return printable_size;
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int i = 0; i < printable_size.rows; ++i){
        for(int j = 0; j < printable_size.cols; ++j){
            output << ((j != 0) ? "\t"s : ""s);
            if(sheet_.count(Position({i, j}))){
                auto val = sheet_.at(Position({i, j})).GetValue();
                if(std::holds_alternative<double>(val)){
                    output << std::get<double>(val);
                }
                else if(std::holds_alternative<FormulaError>(val)){
                    output << std::get<FormulaError>(val);
                }
                else{
                    output << std::get<std::string>(std::move(val));
                }
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(int i = 0; i < printable_size.rows; ++i){
        for(int j = 0; j < printable_size.cols; ++j){
            output << ((j != 0) ? "\t"s : ""s);
            if(sheet_.count(Position({i, j}))){
                output << sheet_.at(Position({i, j})).GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}