#include "cell.h"

#include <cassert>
#include <variant>
#include <algorithm>

Cell::Impl::Impl(){}

Cell::Impl::Impl(SheetInterface* sheet, Position pos)
: sheet_(sheet), pos_(pos){}

Cell::Value Cell::Impl::GetValue() const{
    return GetText();
}

std::string Cell::Impl::GetText() const{
    return "";
}

std::vector<Position> Cell::Impl::GetReferencedCells() const{
    return children;
}

SheetInterface* Cell::Impl::GetSheet() const{
    return sheet_;
}
    
Position Cell::Impl::GetPosition() const{
    return pos_;
}

void Cell::Impl::ResetCashe() const{
    cache.reset();
    for(const Position& parent : parents){
        sheet_->GetCell(parent)->ResetCashe();
    }
}

void Cell::Impl::FaultChildren() const{
    for(const Position& child : children){
        sheet_->GetCell(child)->DeleteParent(pos_);
    }
}

void Cell::Impl::ChangePosition(Position new_pos){
    ResetCashe();
    FaultChildren();
    pos_ = new_pos;
}

void Cell::Impl::DeleteParent(const Position par_pos){
    parents.erase(std::find(parents.begin(), parents.end(), par_pos));
}

bool Cell::Impl::IsCycleSearch(const Position find_pos, const Position parent_pos){
    if(find_pos == pos_){
        return true;
    }
    for(const Position& child : children){
        if(sheet_->GetCell(child)->IsCycleSearch(find_pos, Position::NONE)){
            return true;
        }
    }
    if(!(parent_pos == Position::NONE)){
        parents.push_back(parent_pos);
    }
    return false;
}

Cell::TextImpl::TextImpl(SheetInterface* sheet, Position pos, std::string&& text)
: Impl(sheet, pos)
, text_(std::move(text)){}

Cell::TextImpl::TextImpl(Impl* base_impl, std::string&& text)
: Impl(base_impl->GetSheet(), base_impl->GetPosition())
, text_(std::move(text)){}

Cell::Value Cell::TextImpl::GetValue() const {
    return (text_[0] == '\'') ? text_.substr(1) : GetText();
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(Impl* base_impl, std::string&& formula) 
: Impl(base_impl->GetSheet(), base_impl->GetPosition())
, formula_(ParseFormula(std::move(formula))){
    children = formula_->GetReferencedCells();
    for(const Position& child : children){
        if(sheet_->GetCell(child) == nullptr){
            sheet_->SetCell(child, "");
        }
        if(sheet_->GetCell(child)->IsCycleSearch(pos_, pos_)){
            throw CircularDependencyException("Eror: circular dependency");
        }
    }
}

Cell::FormulaImpl::FormulaImpl(SheetInterface* sheet, Position pos, std::string&& formula) 
: Impl(sheet, pos)
, formula_(ParseFormula(std::move(formula))){
    children = formula_->GetReferencedCells();
    for(const Position& child : children){
        if(sheet_->GetCell(child)->IsCycleSearch(pos_, pos_)){
            throw CircularDependencyException("Eror: circular dependency");
        }
    }
}

Cell::Value Cell::FormulaImpl::GetValue() const{
    if(!cache.has_value()){
        cache = formula_->Evaluate(*sheet_);
    }
    if(std::holds_alternative<double>(cache.value())){
        return std::get<double>(cache.value());
    }
    else{
        return std::get<FormulaError>(cache.value());
    }
}

std::string Cell::FormulaImpl::GetText() const{
    return '=' + formula_->GetExpression();
}


Cell::Cell() : impl_(std::make_unique<Impl>()){}
Cell::Cell(SheetInterface* sheet, Position pos) : impl_(std::make_unique<Impl>(sheet, pos)){}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    ResetCashe();
    impl_->FaultChildren();
    if((text[0] == '=') && (text.size() > 1)){
        Impl* form = new FormulaImpl(impl_.get(), std::move(text.substr(1)));
        impl_.reset(form);
    }
    else{
        impl_.reset(new TextImpl(impl_.get(), std::move(text)));
    }
}

void Cell::Clear() {
    ResetCashe();
    impl_->FaultChildren();
    impl_.reset(new Impl(impl_->GetSheet(), impl_->GetPosition()));
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const{
    return impl_->GetReferencedCells();
}

void Cell::ResetCashe() const{
    impl_->ResetCashe();
}

void Cell::ChangePosition(Position new_pos){
    impl_->ChangePosition(new_pos);
}

bool Cell::IsCycleSearch(const Position find_pos, const Position parent_pos) const{
    return impl_->IsCycleSearch(find_pos, parent_pos);
}

void Cell::DeleteParent(const Position par_pos){
    impl_->DeleteParent(par_pos);
}

Cell& Cell::operator=(Cell&& other){
    impl_.swap(other.impl_);
    other.Clear();
    return *this;
}