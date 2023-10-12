#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <memory>
#include <utility>

class Sheet : public SheetInterface {
public:
    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;



private:
    std::vector<std::vector<std::unique_ptr< CellInterface >>> table_;
    Size print_area_;

    void PrintValue(const CellInterface::Value& value, std::ostream& output) const;
    void CorrectPrintArea();
    
};