#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;


//EmptyImpl-----------------------
CellInterface::Value EmptyImpl::GetValue() const {
	return std::string();
}

std::string EmptyImpl::GetText() const {
	return std::string();
}

void EmptyImpl::Clear() {}


//TextImpl------------------------
TextImpl::TextImpl(std::string text) {
	if (text.size() == 0) {
		text_ = ""s;
	}
	else {
		text_ = text;
	}
}

CellInterface::Value TextImpl::GetValue() const {
	if (text_.at(0) == '\'') {
		return text_.substr(1);
	}
	return text_;
}

std::string TextImpl::GetText() const {
	return text_;
}

void TextImpl::Clear() {
	text_.clear();
}


//FormulaImpl---------------------
FormulaImpl::FormulaImpl(const std::string& formula, const SheetInterface& sheet) : formula_(ParseFormula(formula)), sheet_(sheet) {}

CellInterface::Value FormulaImpl::GetValue() const {
	CellInterface::Value buffer;
	auto result = formula_->Evaluate(sheet_);

	if (std::holds_alternative<double>(result)) {
		buffer = std::get<double>(result);
	}
	else {
		buffer = std::get<FormulaError>(result);
	}

	return buffer;
}

std::string FormulaImpl::GetText() const {
	return  std::string("="s) + formula_->GetExpression();
}

void FormulaImpl::Clear() {
	formula_ = ParseFormula(""s);
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}


//Cell----------------------------
Cell::Cell(Position pos, SheetInterface& sheet) :sheet_(&sheet), current_position_(pos){}

void Cell::Set(std::string text) {

	if (text.size() > 0) {
		if (text.at(0) == '=' && text.size() > 1) {
			auto buffer = std::make_unique<FormulaImpl>(text.substr(1), *sheet_);

			std::vector<Position> reference = buffer->GetReferencedCells();

			for (const auto& pos : reference) {
				if (!pos.IsValid()) {
					throw FormulaException("Wrong adress");
				}
			}

			CheckCyclicDependencies(reference);

			forward_positions_ = std::move(reference);
			impl_ = std::move(buffer);
		}
		else {
			impl_ = std::make_unique<TextImpl>(text);
		}
	}
	else {
		impl_ = std::make_unique<EmptyImpl>();
	}

	InvalidateCache();
	InformChilds();
}

void Cell::Clear() {
	impl_->Clear();

	InvalidateCache();
}

Cell::Value Cell::GetValue() const {
	if (HasCache()) {                             
		return cache_.value();                    
	}

	auto value = impl_->GetValue();               

	if (std::holds_alternative<double>(value)) {  
		cache_ = std::get<double>(value);
	}

	return value;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

bool Cell::HasCache() const {
	return cache_.has_value();
}

void Cell::ClearCache() {
	cache_.reset();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return forward_positions_;
}

std::vector<Position> Cell::GetCacheCells() const {
	return backward_positions_;
}

void Cell::InvalidateCache() {
	if (backward_positions_.size() != 0) {                                
		for (const auto& cell_pos : backward_positions_) {                
			Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(cell_pos));  
			cell->ClearCache();                                           
			cell->InvalidateCache();                                      
		}
	}
}

void Cell::InformChilds() {
	if (forward_positions_.size() != 0) {
		for (const auto& cell_pos : forward_positions_) {
			Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(cell_pos));
			cell->AddCacheCell(current_position_);
		}
	}
}

void Cell::AddCacheCell(Position pos) {
	backward_positions_.push_back(pos);
	std::sort(backward_positions_.begin(), backward_positions_.end());
}

void Cell::CheckCyclicDependencies(std::vector<Position>& cells) const {

	for (auto& cell_pos : cells) {
		if (cell_pos == current_position_) {
			throw CircularDependencyException("circular dependency");
		}

		Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(cell_pos));

		if (!cell) {
			sheet_->SetCell(cell_pos, "");
			cell = dynamic_cast<Cell*>(sheet_->GetCell(cell_pos));
		}

		cell->CyclicChecker(current_position_);
	}
}

void Cell::CyclicChecker(Position checked_position) const {

	if (forward_positions_.size() != 0) {                                 
		for (const auto& cell_pos : forward_positions_) {                 
			if (cell_pos == checked_position) {                           
				throw CircularDependencyException("circular dependency"); 
			}

			Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(cell_pos));  

			cell->CyclicChecker(checked_position);                        
		}
	}
}
