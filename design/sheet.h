#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const Cell* GetCell(Position pos) const override;
    Cell* GetCell(Position pos) override;

    void ClearCell(Position pos) override;
 
    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    class HashPosition{
    public:
        int operator()(const Position& pos) const{
            return pos.col + pos.row * (Position::MAX_COLS + 1);
        }
    };

	std::unordered_map<Position, Cell, HashPosition> sheet_;
    Size printable_size = {0, 0};
    std::unordered_map<size_t, int> count_rows;
    std::unordered_map<size_t, int> count_cols;
};