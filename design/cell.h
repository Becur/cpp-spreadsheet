#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <string>
#include <memory>
#include <vector>
#include <optional>

class Cell : public CellInterface {
public:
    Cell(Sheet* sheet_, Position pos);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void ResetCashe() const;

    void ChangePosition(Position new_pos);

    bool IsCycleSearch(const Position find_pos, const Position parent_pos) const;

    void DeleteParent(const Position par_pos);

private:
    class Impl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
};

class Cell::Impl{
public:
    Impl(Sheet* sheet, Position pos);

    virtual Cell::Value GetValue() const;

    virtual std::string GetText() const;

    std::vector<Position> GetReferencedCells() const;

    void ResetCashe() const;

    void FaultChildren() const;

    void ChangePosition(Position new_pos);

    void DeleteParent(const Position par_pos);

    bool IsCycleSearch(const Position find_pos, const Position parent_pos);

    Sheet* GetSheet() const;

    Position GetPosition() const;
protected:
    Sheet* sheet_;
    std::vector<Position> children;
    std::vector<Position> parents;

    mutable std::optional<FormulaInterface::Value> cache;
    Position pos_;
};


class Cell::TextImpl : public Cell::Impl{
public:
    TextImpl(Impl* base_impl, std::string&& text);

    TextImpl(Sheet* sheet, Position pos, std::string&& text);

    Cell::Value GetValue() const override;

    std::string GetText() const override;

private:
    std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl{
public:
    FormulaImpl(Impl* base_impl, std::string&& text);

    FormulaImpl(Sheet* sheet, Position pos, std::string&& formula);

    Cell::Value GetValue() const override;

    std::string GetText() const override;

    void Addchildren();

private:
    std::unique_ptr<FormulaInterface> formula_;
};
