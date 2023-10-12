#pragma once

#include <optional>
#include <algorithm>

#include "common.h"
#include "formula.h"

class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;

    virtual std::string GetText() const = 0;

    virtual void Clear() = 0;
};


class EmptyImpl : public Impl {
public:
    EmptyImpl() = default;

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

};


class TextImpl : public Impl {
public:
    TextImpl(std::string text);

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

private:
    std::string text_;
};


class FormulaImpl : public Impl {
public:
    FormulaImpl(const std::string& formula, const SheetInterface& sheet);

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

    std::vector<Position> GetReferencedCells() const;

private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_;
};



class Cell : public CellInterface {
public:
    Cell(Position pos, SheetInterface& sheet);

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    std::vector<Position> GetCacheCells() const;

private:
    std::unique_ptr<Impl> impl_;
    std::vector<Position> forward_positions_;
    std::vector<Position> backward_positions_;
    SheetInterface* sheet_ = nullptr;
    mutable std::optional<double> cache_;
    Position current_position_;
  
    void CheckCyclicDependencies(std::vector<Position>& cells) const;
    void CyclicChecker(Position checked_position) const;

    void InvalidateCache();
    void InformChilds();

    bool HasCache() const;
    void AddCacheCell(Position pos);
    void ClearCache();
};