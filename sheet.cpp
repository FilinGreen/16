#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;


void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (table_.size() < size_t(pos.row + 1)) {
        int delta = pos.row + 1 - table_.size();
        for (int i = 0; i < delta; ++i) {
            table_.push_back(std::vector<std::unique_ptr< CellInterface>>());
        }
    }

    if (table_.at(pos.row).size() < size_t(pos.col + 1)) {
        int delta = pos.col + 1 - table_.at(pos.row).size();
        for (int i = 0; i < delta; ++i) {
            table_.at(pos.row).push_back(nullptr);
        }
    }

    std::unique_ptr < CellInterface > cell = std::make_unique<Cell>(pos, *this);
    cell->Set(text);
    table_.at(pos.row).at(pos.col) = std::move(cell);

    if (print_area_.rows < pos.row + 1) {
        print_area_.rows = pos.row + 1;
    }
    if (print_area_.cols < pos.col + 1) {
        print_area_.cols = pos.col + 1;
    }
}


const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (table_.size() < size_t(pos.row + 1) || table_.at(pos.row).size() < size_t(pos.col + 1)) {
        return nullptr;
    }

    return table_.at(pos.row).at(pos.col).get();
}


CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (table_.size() < size_t(pos.row + 1) || table_.at(pos.row).size() < size_t(pos.col + 1)) {
        return nullptr;
    }

    return table_.at(pos.row).at(pos.col).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (table_.size() < size_t(pos.row + 1) || table_.at(pos.row).size() < size_t(pos.col + 1)) {
        return;
    }

    table_.at(pos.row).at(pos.col).release();
    CorrectPrintArea();
}


void Sheet::CorrectPrintArea() {
    size_t maxrow = 0;
    size_t maxcol = 0;

    for (size_t i = 0; i < table_.size(); ++i) {
        size_t size = 0;
        for (size_t n = 0; n < table_.at(i).size(); ++n) {
            if (table_.at(i).at(n) != nullptr) {
                size = n + 1;
            }
        }

        if (size != 0) {
            maxrow = i + 1;
            if (maxcol < size) {
                maxcol = size;
            }

        }
    }

    print_area_.rows = maxrow;
    print_area_.cols = maxcol;
}


Size Sheet::GetPrintableSize() const {
    return print_area_;
}


void Sheet::PrintValues(std::ostream& output) const {
    if (print_area_.rows != 0 && print_area_.cols != 0) {

        for (int row = 0; row < print_area_.rows; ++row) {
            for (int col = 0; col < print_area_.cols; ++col) {

                const CellInterface* cell = GetCell({ row,col });

                if (col != 0) {
                    output << "\t";
                }

                if (cell) {
                    const auto& value = cell->GetValue();
                    PrintValue(value, output);
                }
            }
            output << "\n";
        }
    }
}

void Sheet::PrintValue(const CellInterface::Value& value, std::ostream& output) const {
    if (std::holds_alternative<double>(value)) {
        output << std::get<double>(value);
    }
    else if (std::holds_alternative<std::string>(value)) {
        output << std::get<std::string>(value);
    }
    else {
        output << std::get<FormulaError>(value);
    }
}


void Sheet::PrintTexts(std::ostream& output) const {
    if (print_area_.rows != 0 && print_area_.cols != 0) {
        for (int row = 0; row < print_area_.rows; ++row) {
            for (int col = 0; col < print_area_.cols; ++col) {

                const CellInterface* cell = GetCell({ row,col });

                if (col != 0) {
                    output << "\t";
                }

                if (cell) {
                    output << table_.at(row).at(col)->GetText();
                }
            }
            output << "\n";
        }
    }

}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}